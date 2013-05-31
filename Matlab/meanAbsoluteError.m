function [ distances ] = meanAbsoluteError(vidObj)

% Read Frames in Grayscale format
numFrames = vidObj.NumberOfFrames-2;
bw = uint8(zeros(vidObj.Height, vidObj.Width, numFrames));
for i=1:numFrames
   image = rgb2gray(read(vidObj,i+2));
   bw(:,:,i) = image;
end

% Flatten image into one row
flattened = zeros(numFrames,vidObj.Height*vidObj.Width);
for i=1:numFrames
   frame = bw(:,:,i);
   frame = reshape(frame, 1, vidObj.Height*vidObj.Width);
   flattened(i,:) = frame;
end

% Calculate euclidian distance between observations
grid = squareform(pdist(flattened));

distances = zeros(numFrames-1,1);

for i=2:numFrames
   distances(i-1) = grid(i-1,i); 
end


end

