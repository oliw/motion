function [ original ] = getForwardTransform( original )

point = [0 0 1];
points = point;

for i=1:length(original)
  original{i} = [original{i}; 0 0 1]; 
  original{i} = inv(original{i});
  point = (original{i} * point')'; 
  points = [points; point];
end

time = 0:length(original);

plot(points(:,1), time', points(:,2), time');
xlabel('Movement')
ylabel('Time')
end