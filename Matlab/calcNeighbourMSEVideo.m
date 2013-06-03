function [ mses ] = calcNeighbourMSEVideo( videoFile )

hVideoSrc = vision.VideoFileReader(videoFile, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');
reset(hVideoSrc);

mses = [];

prev = step(hVideoSrc);
while ~isDone(hVideoSrc)
    curr = step(hVideoSrc);
    mses = [mses calcMSE(prev, curr)];
    prev = curr;
end

release(hVideoSrc);

end

