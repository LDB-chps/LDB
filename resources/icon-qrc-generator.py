import sys
from os import listdir
from os.path import isfile, isdir, join, abspath, basename

output = open("resources/icons.qrc", "w")
output.write("<RCC>\n")
output.write("    <qresource prefix=\"/\">\n")
output.write("        <file alias =\"combinear.qss\">" + abspath(sys.argv[1] + "/Combinear.qss") + "</file>\n")
files = [sys.argv[1] + "/icons"]

for f in files:
    if isfile(f):
        output.write("        <file alias=\"icons/" + basename(f) + "\">" + abspath(f) + "</file>\n")
    elif isdir(f):
        for g in listdir(f):
            files.append(join(f, g))

output.write("    </qresource>\n")
output.write("</RCC>\n")

output.close()
