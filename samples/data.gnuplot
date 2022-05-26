# Title
set title 'GPX2video Telemetry'

set grid

set xdata time
set timefmt "%s"
set format x "%H:%M:%S"
set xlabel "Time"

set datafile commentschars "#"

plot "data.csv" u 3:5 w lp t 'lat'
plot "data.csv" u 3:6 w lp t 'lon'
plot "data.csv" u 3:7 w lp t 'ele'

pause mouse close

