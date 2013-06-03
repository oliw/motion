function [ desiredPath ] = motionGraphDrawer(frameTransforms, varargin)
%MOTIONGRAPHDRAWER Draw graphs from Data
 
p = inputParser;
addRequired(p,'frameTransforms', @iscell);
addOptional(p,'updateTransforms', cell(0), @iscell);
addOptional(p,'showOriginal', true, @islogical);
addOptional(p,'showNew', true, @islogical);
addOptional(p,'showX', true, @islogical);
addOptional(p,'showY', true, @islogical);
parse(p,frameTransforms,varargin{:});

frameTransforms = p.Results.frameTransforms;
showNew = ~isempty(p.Results.updateTransforms);
showOriginal = p.Results.showOriginal;
showBoth = showNew && showOriginal;
showX = p.Results.showX;
showY = p.Results.showY;

frameTransforms = make3by3(frameTransforms);

% Build Ct
originalPath = cell(length(frameTransforms),1);
prev = eye(3); 
for t=1:length(frameTransforms)
   path = prev * frameTransforms{t};
   originalPath{t} = path;
   prev = path;
end
% Post Ct = C1C2C3..Ct

if showNew
    % Build Pt
    updateTransforms = p.Results.updateTransforms;
    updateTransforms = make3by3(updateTransforms);
    desiredPath = cell(size(originalPath));
    for t=1:length(frameTransforms)
        desiredPath{t} = originalPath{t} * updateTransforms{t};
    end
end

% Invert Paths
originalPath = invertArrs(originalPath);
if showNew
   desiredPath = invertArrs(desiredPath); 
end

% Build DataSet for Original Data
point = [0 0 1];
originalPoints = zeros(length(frameTransforms)+1,3);
originalPoints(1,:) = point;
for i=1:length(frameTransforms)
  newpoint = (originalPath{i} * point')'; 
  originalPoints(i+1,:) = newpoint;
end

if showNew
    % Build DataSet for New Data
    newPoints = zeros(length(frameTransforms)+1,3);
    newPoints(1,:) = point;
    for i=1:length(frameTransforms)
      newpoint = (desiredPath{i} * point')';
      newPoints(i+1,:) = newpoint;
    end
end

% The origin in the video is the top left corner.
% We need to flip the Y coords

time = 0:length(frameTransforms);
if showBoth
    if showX
        figure;
        plot(originalPoints(:,1), time', newPoints(:,1), time');
        title('Movement in X Direction');
        xlabel('Movement');
        ylabel('Time');
        legend('Original Motion','New Motion');
    end
    
    if showY 
        figure;
        plot(originalPoints(:,2), time', -1*newPoints(:,2), time');
        title('Movement in Y Direction');
        xlabel('Movement');
        ylabel('Time');
        legend('Original Motion','New Motion');
    end
    
else
    if showNew
       data = newPoints;
       legendText = 'New Motion';
    else
       data = originalPoints;
       legendText = 'Original Motion';
    end
    if showX
        figure;
        plot(data(:,1), time');
        title('Movement in X Direction');
        xlabel('Movement');
        ylabel('Time');
        legend(legendText);
    end
    
    if showY 
        figure;
        plot(-1*data(:,2), time');
        title('Movement in Y Direction');
        xlabel('Movement');
        ylabel('Time');
        legend(legendText);
    end            
end

end

function [ cellArr ] = make3by3 (cellArr)
% Convert from 2x3 to 3x3    
for t=1:length(cellArr)
    cellArr{t} = [cellArr{t}; 0 0 1];
end

end

function [ cellArr ] = invertArrs (cellArr) 
for t=1:length(cellArr)
    cellArr{t} = inv(cellArr{t});
end
end