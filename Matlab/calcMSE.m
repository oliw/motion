function [ mse ] = calcMSE(image1, image2)
%CALCMSE
differences = (image1 - image2).^2;
mse = mean2(differences);

end

