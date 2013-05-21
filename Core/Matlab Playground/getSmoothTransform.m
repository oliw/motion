function [ result ] = getSmoothTransform( original, update )

for i=1:length(update)
   update{i} = [update{i}; 0 0 1]; 
end

% Get original movement
point = [0 0 1];
points = point;
for i=1:length(original)
  point = (original{i} * point')'; 
  points = [points; point];
end

% Apply updates
for i=2:length(points)
  points(i,:) = (update{i-1} * points(i,:)')';

time = 0:length(original);

plot(points(:,1), time', points(:,2), time');
xlabel('Movement');
ylabel('Time');

end

