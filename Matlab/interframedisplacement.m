function [ result ] = interframedisplacement( points, method )
%INTERFRAMEDISPLACEMENT Summary of this function goes here
%   Detailed explanation goes here

grid = squareform(pdist(points,method));

result = zeros(length(points)-1,1);

for i=2:length(points)
   result(i-1) = grid(i-1,i); 
end

end

