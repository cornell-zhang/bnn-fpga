#!/usr/bin/env python

import os, re, glob, sys
import xml.etree.ElementTree as ET

#-------------------------------------------------------------------------
# Helper functions
#-------------------------------------------------------------------------
# get all the immediate directories
def get_immediate_subdirs(dir):
  return filter(os.path.isdir, [os.path.join(dir,f) for f in os.listdir(dir)])

# Safely change directories. Uncommet prints for debugging
def changedir(dir):
  #print >>sys.stderr, "Entering", dir
  if not os.path.exists(dir):
    #print >>sys.stderr, "Path", dir, "does not exist"
    return 0
  os.chdir(dir)
  return 1

# Find text on an xml node
def find_text(node, string):
  newnode = node.find(string)
  if not newnode is None:
    return newnode.text
  else:
    #print >>sys.stderr, "Could not find", string
    return "-not found-"

#-------------------------------------------------------------------------
# Parse resource
#-------------------------------------------------------------------------
def parse_impl_xml(file):
  tree = ET.parse(file)
  root = tree.getroot()

  # area report
  area_report = root.find("AreaReport")
  resources = area_report.find("Resources")
  slice = resources.find("SLICE").text
  lut = resources.find("LUT").text
  ff = resources.find("FF").text
  srl = resources.find("SRL").text
  bram = resources.find("BRAM").text
  dsp = resources.find("DSP").text

  # timing report
  timing_report = root.find("TimingReport")
  target_cp = timing_report.find("TargetClockPeriod").text
  actual_cp = timing_report.find("AchievedClockPeriod").text

  return dict([("Slice",slice), ("LUT",lut), ("FF",ff), \
    ("SRL",srl), ("BRAM",bram), ("DSP",dsp), \
    ("Target-CP",target_cp), ("Actual-CP",actual_cp)])

def parse_impl():
  TopLevel = os.getcwd()
  res_dir = "impl/report/verilog/"
  if not changedir(res_dir):
    print >>sys.stderr, "**** Cannot find", res_dir, "in", TopLevel
    return 0

  files = glob.glob("*.xml")
  if len(files) != 1:
    files.sort(key=len)
    #print >>sys.stderr, "Found", len(files), "xml files in"
    #print >>sys.stderr, "\t", TopLevel++"/"+res_dir
    #print >>sys.stderr, "expected 1"
    #return 0

  print "Parsing", files[0]
  results = parse_impl_xml(files[0])
  os.chdir(TopLevel)
  return results

#-------------------------------------------------------------------------
# Parse resource
#-------------------------------------------------------------------------
def parse_syn_xml(file):
  tree = ET.parse(file)
  root = tree.getroot()

  name = root.find("UserAssignments").find("TopModelName").text
  ver = root.find("ReportVersion").find("Version").text
  device = root.find("UserAssignments").find("Part").text
  summary = root.find("PerformanceEstimates").find("SummaryOfOverallLatency")
  # these may not exist if design is not pipelined
  avg_lat = find_text(summary,"Average-caseLatency")
  worst_lat = find_text(summary,"Worst-caseLatency")
  actual_II = find_text(summary,"PipelineInitiationInterval")
  depth = find_text(summary,"PipelineDepth")
  
  timing = root.find("PerformanceEstimates").find("SummaryOfTimingAnalysis")
  period = timing.find("EstimatedClockPeriod").text

  res = root.find("AreaEstimates").find("Resources")
  lut = res.find("LUT").text
  ff = res.find("FF").text
  bram = res.find("BRAM_18K").text

  return dict([("Name",name), ("Version",ver), ("Device",device), \
    ("Avg-Lat",avg_lat), ("Worst-Lat",worst_lat), ("Actual-II",actual_II), ("Depth",depth), \
    ("Est-CLK",period), ("Est-LUT",lut), ("Est-FF",ff), ("Est-BRAM",bram)])

def parse_syn():
  TopLevel = os.getcwd()
  dir = "syn/report/"
  if not changedir(dir):
    print >>sys.stderr, "**** Cannot find", dir, "in", TopLevel
    return 0

  files = glob.glob("*.xml")
  if len(files) != 1:
    if re.match(".*\/rs", TopLevel):
      files = ["rs_decode_csynth.xml"]
    else:
      files.sort(key=len)
      #print >>sys.stderr, "Found", len(files), "xml files in"
      #print >>sys.stderr, "\t", TopLevel+"/"+dir
      #print >>sys.stderr, "expected 1"
      #return 0

  print "Parsing", files[0]
  results = parse_syn_xml(files[0])
  os.chdir(TopLevel)
  return results

#-------------------------------------------------------------------------
# Parse sim
#-------------------------------------------------------------------------
def parse_sim(soln_dir):
  TopLevel = os.getcwd()
  dir = "sim/report/"
  if not changedir(soln_dir+"/"+dir):
    print >>sys.stderr, "**** Cannot find", dir, "in", TopLevel+"/"+soln_dir
    return 0

  files = glob.glob("*cosim.rpt")
  if len(files) != 1:
    files.sort(key=len)

  print "Parsing", files[0]
  f = open(files[0], 'r')
  data = []

  for line in f:
    if re.match("\|\W+Verilog\|\W+Pass\|", line):
      m = re.search("\|\W+(\d+)\|\W+(\d+)\|\W+(\d+)\|", line)
      if m:
        data = [m.group(1), m.group(2), m.group(3)]

  f.close()

  if len(data) == 0:
    print "Cannot find Verilog sim results"
  os.chdir(TopLevel)
  return data

#-------------------------------------------------------------------------
# Parse both the impl report and the syn report
#-------------------------------------------------------------------------
def parse_impl_and_syn(soln_dir):
  SolnLevel = os.getcwd()
  data = dict([])
  success = False
  
  if changedir(soln_dir):
    # parse the syn xml file
    syn = parse_syn()
    if syn:
      data = dict(data.items()+syn.items())
      success = True
    
    # parse the impl xml file
    impl = parse_impl()
    if impl:
      data = dict(data.items()+impl.items())
      success = True

    cosim = parse_sim

  else:
    print >>sys.stderr, "Cannot find solution", soln_dir
    return 0
  
  os.chdir(SolnLevel)
  
  if not success:
    return 0
  return data;

#-------------------------------------------------------------------------
# Function to process a single HLS project directory
#-------------------------------------------------------------------------
def process_project(prj_dir):
  TopLevel = os.getcwd()

  # enter the project directory
  if not changedir(prj_dir):
    print >>sys.stderr, "Cannot find project", prj_dir
    exit(-1)

  # search for solution subdirs
  solutions = get_immediate_subdirs('.')
  
  # for each solution, parse the data
  for sol in solutions:
    data = parse_impl_and_syn(sol)
    sdata = parse_sim(sol)
    
    if data:
      print "\nSolution: ", sol
      print_data(data)

    if sdata:
      print "\n** Sim. Report **"
      print "Min:", sdata[0]
      print "Avg:", sdata[1]
      print "Max:", sdata[2]

  os.chdir(TopLevel)

#-------------------------------------------------------------------------
# Print dictionary data
#-------------------------------------------------------------------------
def print_dict(d, key):
  if key in d:
    print "%-12s: %-30s" % (key, d[key])
  else:
    print "%-12s: %-30s" % (key, "-not found-")

def print_data(d):
  print "** Syn. Report **"
  print_dict(d, "Name")
  print_dict(d, "Version")
  print_dict(d, "Device")
  print_dict(d, "Avg-Lat")
  print_dict(d, "Worst-Lat")
  print_dict(d, "Actual-II")
  print_dict(d, "Depth")
  print_dict(d, "Est-CLK")
  print_dict(d, "Est-LUT")
  print_dict(d, "Est-FF")
  print_dict(d, "Est-BRAM")
  print ""
  print "** Impl. Report **"
  print_dict(d, "Target-CP")
  print_dict(d, "Actual-CP")
  print_dict(d, "Slice")
  print_dict(d, "LUT")
  print_dict(d, "FF")
  print_dict(d, "SRL")
  print_dict(d, "BRAM")
  print_dict(d, "DSP")

#-------------------------------------------------------------------------
# Main function
#-------------------------------------------------------------------------
def main():
  if len(sys.argv) == 1:
    print >>sys.stderr, "Usage: parse_vivado.py <hls project dir>"
  else:
    dir = sys.argv[1]
    process_project(dir)
    print ""

if __name__ == "__main__":
  main()

#print  >>sys.stderr, "Current directory: ", os.getcwd(), "\n"
#print "name,", "version,", "device,"
#print "target_cp\t", "actual_cp\t", "latency\t", \
#    "slice\t", "lut\t", "ff\t", "srl\t" \
#    "binvars\t", "intvars\t", "constraints\t", "runtime(s)\t"

#Dirs = get_immediate_subdirs(".")

