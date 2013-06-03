function [ movMean ] = calcMeanImage( videoFile )
%CALCMEANIMAGE Summary of this function goes here
%   Detailed explanation goes here
hVideoSrc = vision.VideoFileReader(videoFile, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');
% Reset the video source to the beginning of the file.
reset(hVideoSrc);

% Process all frames in the video
movMean = step(hVideoSrc);
ii = 2;
while ~isDone(hVideoSrc)
    % Read in new frame
    movMean = movMean + step(hVideoSrc);
    ii = ii+1;
end
movMean = movMean/(ii-2);

release(hVideoSrc);
end


