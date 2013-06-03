function [ psnrs ] = calcRefPSNRVideo( videoFile )

hVideoSrc = vision.VideoFileReader(videoFile, 'ImageColorSpace', 'Intensity', 'VideoOutputDataType','double');
reset(hVideoSrc);

psnrs = [];

ref = step(hVideoSrc);
curr = ref;
psnrs = [psnrs calcMSE(curr, ref)];
hpsnr = vision.PSNR;
while ~isDone(hVideoSrc)
    curr = step(hVideoSrc);
    psnrs = [psnrs step(hpsnr, curr, ref)];
end

release(hVideoSrc);

end

