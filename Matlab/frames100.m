function frames100( input, output )
%VIDEOREADER Summary of this function goes here
%   Detailed explanation goes here

readerObj = VideoReader(input);
writerObj = VideoWriter(output);
writerObj.Quality = 100;

open(writerObj);

for k = 1:50
   img = read(readerObj,k);
   writeVideo(writerObj,img);
end

close(writerObj);

end

