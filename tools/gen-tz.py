#!/usr/bin/env python
# Python 3.6+

import csv
from collections import defaultdict
from dataclasses import dataclass
from urllib.request import urlopen
from codecs import iterdecode

@dataclass
class DataItem():
    area: str
    location: str
    tz: str
    index: int

tz_data_url = 'https://raw.githubusercontent.com/nayarsystems/posix_tz_db/master/zones.csv'
tzs = []
areas = defaultdict(list) # Dict[area, DataItem]
longest_string = 0

f = urlopen(tz_data_url)
reader = csv.DictReader(iterdecode(f, 'utf8'), fieldnames=('name', 'tz'))

for row in reader:
    longest_string = max(len(row['name']), longest_string)
    area, location = row['name'].split('/', 1)

    if row['tz'] not in tzs:
        tzs.append(row['tz'])

    idx = tzs.index(row['tz'])
    di = DataItem(area, location, row['tz'], idx)
    areas[area.upper()].append(di)

output = '''
// This file is generated with gen-tz.py

'''

output += 'uint8_t longest_string = {};\n'.format(longest_string)

for area in sorted(areas.keys()):
    for i, di in enumerate(areas[area]):
        output += 'const char {area}_{i}[] PROGMEM = "{value}";\n'.format(area=area, i=i, value=di.location)

print(output)
