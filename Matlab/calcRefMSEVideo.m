function [ mses ] = calcRefMSEVideo( videoFile )

hVideoSrc = vision.VideoFileReader(videoFile, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');
reset(hVideoSrc);

mses = [];

ref = step(hVideoSrc);
curr = ref;
mses = [mses calcMSE(curr, ref)];
while ~isDone(hVideoSrc)
    curr = step(hVideoSrc);
    mses = [mses calcMSE(curr, ref)];
end

release(hVideoSrc);

end
