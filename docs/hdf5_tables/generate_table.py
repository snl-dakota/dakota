#!/usr/bin/env python
# coding: utf-8
import yaml
import re
import html
import sys

left_column_width=2
dataset_widths = [8, 90]

scale_widths = [left_column_width, dataset_widths[0], 8, 8, 12, 38, 24]

def table_heading(name):
    return """<table  class="spec-table">
<tr>
 <th width="%d%%" class="border-heavy-right"></th>
 <th class="border-heavy-right" colspan="6"><b>%s</b> </td>
</tr>""" % (left_column_width, name)

def dataset_row(name, content):
    content = html.escape(content)
    content = re.sub(r'&lt;(.*?)&gt;', r'<i>&lt;\1&gt;</i>', content)
    
    return """<tr>
 <th class="border-heavy-right"></th>
 <td class="border-light-right" width="%d%%">%s</th>
 <td class="border-light-right" width="%d%%" colspan="5">%s</td>
</tr>""" %(dataset_widths[0],name, dataset_widths[1], content)

def scales_heading(length):
    tokens = scale_widths[:]
    tokens.insert(1, length+1)
    tokens.insert(3, length+1)
    return """<tr>
 <th width="%d%%" class="border-heavy-right" rowspan="%d"></th>
 <th width="%d%%" rowspan="%d" valign="middle"><b>Scales</b> </th>
 <th width="%d%%">Dimension </th>
 <th width="%d%%">Type </th>
 <th width="%d%%">Label </th>
 <th width="%d%%">Contents </th>
 <th width="%d%%">Notes </th>
</tr>""" % tuple(tokens)
def scale_row(items):
    try:
        if not items["literal_contents"]:
            items["contents"] = r'<i>%s</i>' % items["contents"]
    except KeyError:
        pass
    try:
        items["notes"]
    except KeyError:
        items["notes"] = ""
    return """<tr>
 <td class="border-light-right">{dimension} </td>
 <td class="border-light-right">{type} </td>
 <td class="border-light-right">{label} </td>
 <td class="border-light-right">{contents}</td>
 <td class="border-light-right">{notes}</td>
</tr>""".format(**items)


if __name__ == "__main__":
    inputfile = sys.argv[1]
    outputfile = sys.argv[2]
    
    with open(inputfile,"r") as f:
        data = yaml.load(f.read())

    with open(outputfile,"w") as f:
        print(table_heading(data["name"]), file=f)
        print(dataset_row("Description", data["dataset"]["description"]), file=f)
        print(dataset_row("Location", data["dataset"]["location"]), file=f)
        try:
            print(dataset_row("Notes", data["dataset"]["notes"]),file=f)
        except KeyError:
            pass
        print(dataset_row("Shape", data["dataset"]["shape"]),file=f)
        print(dataset_row("Type", data["dataset"]["type"]),file=f)
        try:
            print(scales_heading(len(data["scales"])),file=f)
            for items in data["scales"]:
                print(scale_row(items), file=f)
        except KeyError:
            pass
        print(r'</table>', file=f)


