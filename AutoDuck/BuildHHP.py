import os
import sys
import glob

"""
BuildHHP.py

Build HTML Help project file.
"""

sHHPFormat = """
[OPTIONS]
Binary TOC=Yes
Compatibility=1.1 or later
Compiled file=%(output)s.chm
Contents file=%(output)s.hhc
Default Window=Home
Default topic=%(target)s.HTML
Display compile progress=Yes
Full-text search=Yes
Index file=%(output)s.hhk
Language=0x409 English (United States)

[WINDOWS]
Home="%(target)s","%(target)s.hhc","%(target)s.hhk","%(target)s.HTML","%(target)s.HTML",,,,,0x63520,,0x387e,,,,,,2,,0


[FILES]
%(output)s.HTML
%(html_files)s

[INFOTYPES]
"""

def handle_globs(lGlobs):
  lFiles = []
  for g in lGlobs:
    lFiles = lFiles + glob.glob(g)
  # lFiles is now the list of origin files.
  # Normalize all of the paths:
  cFiles = len(lFiles)
  for i in range(cFiles):
    lFiles[i] = os.path.normpath(lFiles[i])
  # If it isn't a file, ignore it.
  i = 0
  while i < cFiles:
    if not os.path.isfile(lFiles[i]):
      del lFiles[i]
      cFiles = cFiles - 1
      continue
    i = i + 1
  # Find the common prefix of all of the files
  sCommonPrefix = os.path.commonprefix(lFiles)
  # Ok, now remove this common prefix from every file:
  lRelativeFiles = []
  for file in lFiles:
    lRelativeFiles.append(file[len(sCommonPrefix):])
  return (lRelativeFiles, lFiles)

def main():
  output = os.path.abspath(sys.argv[1])
  target = sys.argv[2]
  f = open(output + ".hhp", "w")
  html_files = ""
  if len(sys.argv) > 2:
    # sys.argv[3] == html_dir
    # sys.argv[4:] == html_files (globbed)
    html_dir = os.path.abspath(sys.argv[3])
    lGlobs   = sys.argv[4:]
    lDestFiles, lSrcFiles = handle_globs(lGlobs)
    # ensure HTML Help build directory exists.
    try:
      os.makedirs(html_dir)
    except:
      pass
    # copy files into html_dir
    for i in range(len(lDestFiles)):
      file = lDestFiles[i]
      file = os.path.join(html_dir, file)
      # ensure any directories under html_dir get created.
      try:
        os.makedirs(os.path.split(file)[0])
      except:
        pass
      sCmd = "copy %s %s > NUL" % (lSrcFiles[i], file)
      os.system(sCmd)
      
    for file in lDestFiles:
      html_files = html_files + '%s\\%s\n' % (html_dir, file)
    
  f.write(sHHPFormat % { "output" : output, "target" : target,
                         "html_files" : html_files })
  f.close()

if __name__ == "__main__":
  main()

