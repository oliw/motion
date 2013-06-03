function compareMeanImages( movMean, movMean2 )
%COMPAREMEANIMAGES Summary of this function goes here
%   Detailed explanation goes here

figure; imshowpair(movMean, movMean2, 'montage');
title(['Mean of 1st Video', repmat(' ',[1 50]), 'Mean of 2nd Video']);


end

