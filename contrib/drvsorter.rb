def dist(p1, p2)
  return Math.sqrt((p2[0] - p1[0])**2 +(p2[1] - p1[1])**2)
end

f = File.new("/tmp/test.drv", "r")

points = []
while !f.eof?
  line = f.readline()
  (x, y) = line.scan(/(.*),(.*)/)[0]
  x = x.to_f
  y = y.to_f
  points.push([x, y])
end

ppoints = points.clone()

p1 = points.pop
while !points.empty?
  print p1[0], ", ", p1[1], "\n"

  min_dist = 9999999999
  min_dist_i = -1
  points.each_index{|i|
    p2 = points[i]
    if (dist(p1, p2) < min_dist) 
      min_dist = dist(p1, p2)
      min_dist_i = i
    end
  }

  p1 = points[min_dist_i]
  points.delete_at(min_dist_i)
end
print p1[0], ", ", p1[1], "\n"

# EOF #
