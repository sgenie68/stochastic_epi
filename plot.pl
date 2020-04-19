set datafile separator ','
set key autotitle columnhead
#set y2tics
set multiplot 
set size 1,0.5; 

if (!exists("filename")) filename='days.csv'

#plot filename  using 1:2 with lines axis x1y1, '' using 1:3 with lines  axis x1y1, '' using 1:4 with lines  axis x1y1, '' using 1:5 with lines  axis x1y1,'' using 1:6 with lines  axis x1y2, '' using 1:7 with lines  axis x1y2
set origin 0.0,0.5;
plot filename  using 1:3 with lines, '' using 1:4 with lines, '' using 1:5 with lines, '' using 1:6 with lines;
set origin 0.0,0.0;
plot filename  using 1:7 with lines, '' using 1:8 with lines;
unset multiplot 
pause -1
