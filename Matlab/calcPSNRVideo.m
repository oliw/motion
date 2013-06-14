function [ psnrs ] = calcPSNRVideo( videoFile, globalOrInter)

hVideoSrc = vision.VideoFileReader(videoFile, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');
reset(hVideoSrc);

psnrs = [];

first = step(hVideoSrc);
prev = first;

while ~isDone(hVideoSrc)
    curr = step(hVideoSrc);
    if strcmp(globalOrInter,'global')
        ref = first;
    else
        ref = prev;
    end
    psnr = calcPSNR(curr,ref);
    psnrs = [psnrs psnr];
    prev = curr;
end

release(hVideoSrc);

end

function [psnr] = calcPSNR(frame, ref)
mse = calcMSE(frame,ref);
L = 255;
psnr = 10*log10((L^2)/mse);
end