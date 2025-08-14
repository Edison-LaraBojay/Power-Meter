# Backlog

## S0 - Agile setup spike (90 min)
Acceptance:
- BACKLOG.md with Sprint / Product / Done
- LOG.md template refined
- choose WIP limit
- S1-S4 slices written with acceptance + DoD

- DoD:
- I can start S1 immediately after this
- less than 90 minutes spent

## S1 - Cadence
PASS: Walk 60, 100, 160, 180 spm (plus or minus 3); NO double counts
Evidence:
 - /media/cadence/cadence_sixty.png
 - /media/cadence/cadence_hundred.png
 - /media/cadence/cadence_hundredsixty.png
 - /media/cadence/cadence_hundredeighty.png

## S2 - Climb Rate
PASS: Standing drift plus or minus 0.22 meter per minute; Test on neb hill and reitz hill, be within 2% of the actual measured amount (some external source); climb_rate changes within 1 second of changing grade (flat to non flat and vice versa)
Evidence:
 - /media/climb_rate/alt_standing.png
 - /media/climb_rate/neb_hill_climb.png
 - /media/climb_rate/reitz_hill_climb.png
 - add as needed (videos too)

## S3 - Total power
PASS: 
- Log at 10 Hz time, altitude, climb_rate, total_power, and p_vert_proxy for both uphill to flat, downhill to flat, and flat.
- Ensure that total_power exactly increased by the p_vert_proxy during the entire duration of uphill, downhill, and flat.
- When transitioning from downhill/uphill to flat, adjusted power changes within 1 seconds reducing/increasing the total power by the necessary amounnt exactly.
Evidence:
- /media/total_power/flat.png
- /media/total_power/uphill.png
- /media/total_power/downhill.png
- /media/total_power/down_to_flat.png
- /media/total_power/up_to_flat.png
- (add as needed, videos too)

## S4 - LED
PASS: 
- LED changes within 1 second of changing effort; refreshes at a 10 Hz rate; matches color visually agreed upon prior for different zones; gradually changes when increasing/decreasing effort. 
Evidence:
- add as needed
