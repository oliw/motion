function [ points ] = readDisplacements( file )
%READDISPLACEMENTS Summary of this function goes here
%   Detailed explanation goes here

[frame, x, y] = textread(file,'%u %u %u', 'delimiter',',');

points = [x y];

%points(1:2,:) = []; 

points = bsxfun(@minus, points, points(1,:));

%l1distance = sqrt(sum(points.^2,2));

end

