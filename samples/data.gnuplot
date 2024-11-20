# Title
set title 'GPX2video Telemetry'

set grid

set timefmt "%s"
set xlabel "Distance"
#set xlabel "Time"
#set xdata time
#set format x "%H:%M:%S"

set datafile commentschars "#"

plot "data.csv" u 11:9 w lp lc rgb 'blue' t 'elevation', \
     "data.csv" u 11:10 w lp lc rgb 'orange' t 'slope'

#plot "data.csv" u ($1/1000):9 w lp lc rgb 'blue' t 'elevation', \
#     "data.csv" u ($1/1000):10 w lp lc rgb 'orange' t 'slope'

pause mouse close

