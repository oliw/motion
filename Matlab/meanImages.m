function [movMean, movMean2] = meanImages( oldVideo, newVideo )
%MEANIMAGES Summary of this function goes here
%   Detailed explanation goes here

hVideoSrc = vision.VideoFileReader(oldVideo, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');
hVideoSrc2 = vision.VideoFileReader(newVideo, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');

% Reset the video source to the beginning of the file.
reset(hVideoSrc);
reset(hVideoSrc2);

% Process all frames in the video
movMean = step(hVideoSrc);
movMean2 = step(hVideoSrc2);

ii = 2;
while ~isDone(hVideoSrc)
    % Read in new frame
    movMean = movMean + step(hVideoSrc);
    movMean2 = movMean2 + step(hVideoSrc2);
    ii = ii+1;
end
movMean = movMean/(ii-2);
movMean2 = movMean2/(ii-2);

figure; imshowpair(movMean, movMean2, 'montage');
title(['Raw input mean', repmat(' ',[1 50]), 'Corrected sequence mean']);

% Here you call the release method on the objects to close any open files
% and release memory.
release(hVideoSrc);
reset(hVideoSrc2);

end

