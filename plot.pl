set datafile separator ','
set key autotitle columnhead

if (!exists("filename")) filename='days.csv'

plot filename  using 1:2 with lines, '' using 1:3 with lines, '' using 1:4 with lines, '' using 1:5 with lines,'' using 1:6 with lines
pause -1
