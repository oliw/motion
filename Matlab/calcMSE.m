function [ mse ] = calcMSE( image1, image2)
%CALCMSE Summary of this function goes here

differences = (image1 - image2).^2;
mse = mean2(differences);

end

