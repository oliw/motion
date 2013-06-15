function [ maes ] = calcMAEVideo( videoFile, globalOrInter )
%CALCMAE Summary of this function goes here

hVideoSrc = vision.VideoFileReader(videoFile, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');
reset(hVideoSrc);

maes = [];

first = step(hVideoSrc);
prev = first;

while ~isDone(hVideoSrc)
    curr = step(hVideoSrc);
    if strcmp(globalOrInter,'global')
        ref = first;
    else
        ref = prev;
    end
    mae = calcMAE(curr,ref);
    maes = [maes mae];
    prev = curr;
end

release(hVideoSrc);

end

function mae = calcMAE(image1,image2)

mae = mean2(abs(image1-image2));

end

