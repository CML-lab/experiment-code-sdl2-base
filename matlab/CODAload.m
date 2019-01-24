%load  in Codamotion data, parsing the header from the event file and
%  merging it with the raw data

function [data,Dheader,varargout] = CODAload(varargin)
%clear all;
if nargin == 0
    [fname,fpath] = uigetfile('*.*','Select (Event) data file to analyze');
else
    fpath = varargin{1};
    fname = '';
end

if strfind(fname,'E.dat')
    %this is the event file, we will parse it here
    codafname = strrep(fname,'E.dat','.coda');
    if ~exist([fpath codafname],'file')
        %couldn't find the E file, ask for it:
        [codafname] = uigetfile([fpath '*.*'],'Select corresponding CODA file:');
    end
    
elseif strfind(fname,'.coda')
    codafname = fname;
    fname = strrep(fname,'.coda','E.dat');
    if ~exist([fpath fname],'file')
        %couldn't find the E file, ask for it:
        [fname] = uigetfile([fpath '*.*'],'Select corresponding event file:');
    end
end
%now, we have the name of the event file in fname, and the name of the coda file in codafname

fid = fopen([fpath fname],'r');

doloop = 1;
while (doloop)
    temp = fgetl(fid);
    if strcmp(temp,'--')
        %detect the end of the header
        temp = fgetl(fid);
        Eheader.cols = textscan(temp,'%s');
        Eheader.cols = Eheader.cols{1};
        fgetl(fid);
        doloop = 0;
    else
        hvals = textscan(temp,'%s %s');
        if ~isempty(str2num(char(hvals{2})))
            hvals = textscan(temp,'%s %d');
            tmp = strrep(char(hvals{1}),':','');
            eval(sprintf('Eheader.%s = %d;',tmp,hvals{2}));
        else
            tmp = strrep(char(hvals{1}),':','');
            tmp1 = strrep(char(hvals{2}),'.txt','');
            eval(sprintf('Eheader.%s = ''%s'';',tmp,char(tmp1)));
        end
            
    end
end

% %unknown columns, use this code for complete flexiblity
readstr = '';
for a = 1:length(Eheader.cols)
    switch(char(Eheader.cols{a}))
        case {'Device_Num','SampleNum','Trial'}
            readstr = [readstr '%d '];
        case 'TrackerTime'
            readstr = [readstr '%f '];
        case {'HandX','HandY','HandZ','Velocity'}
            readstr = [readstr '%f '];
        case {'StartX','StartY','TargetX','TargetY'}
            readstr = [readstr '%f '];
        case {'Trace','TrType'}
            readstr = [readstr '%d '];
        case 'Keymap'
            readstr = [readstr '%s '];
        otherwise
            readstr = [readstr '%f '];
    end
end

% %known columns, use this code for proper formatting
% readstr = '%d %d %f %f %f %f %f %f %f %d'; %devNo, time, x, y, theta, startx, starty, tgtx, tgty, trace

%doloop = 1;
%while (doloop)
dvals = textscan(fid,readstr,inf);

%compute number of birds
numCODAs = length(unique(dvals{1}));

for a = 1:numCODAs
    
    %divide data structure into appropriate fields, based on column names
    for b = 1:length(Eheader.cols)
        eval(sprintf('event(%d).%s = dvals{%d}(%d:%d:end);',a,char(Eheader.cols{b}),b,a,numCODAs));
    end
    
end %end for loop

fclose(fid);

clear dvals;


%% parse the coda (raw data) file

fid = fopen([fpath codafname],'r');

doloop = 1;
temp = fgetl(fid);  %get rid of the header line
while (doloop)
    temp = fgetl(fid);
    if strcmp(temp,'--')
        %detect the end of the header
        temp = fgetl(fid);
        Dheader.cols = textscan(temp,'%s');
        Dheader.cols = Dheader.cols{1};
        fgetl(fid); %clear the column title separator
        doloop = 0;
    else
        hvals = textscan(temp,'%s %s');
        if ~isempty(str2num(char(hvals{2})))
            hvals = textscan(temp,'%s %d');
            tmp = strrep(char(hvals{1}),':','');
            tmp = strrep(tmp,' ','_');
            tmp = strrep(tmp,'(','');
            tmp = strrep(tmp,')','');
            eval(sprintf('Dheader.%s = %d;',tmp,hvals{2}));
        else
            tmp = strrep(char(hvals{1}),':','');
            tmp1 = strrep(char(hvals{2}),'.txt','');
            eval(sprintf('Dheader.%s = ''%s'';',tmp,char(tmp1)));
        end
            
    end
end

% %unknown columns, use this code for complete flexiblity
readstr = '';
for a = 1:length(Dheader.cols)
    switch(char(Dheader.cols{a}))
        case 'Sample'
            readstr = [readstr '%f '];
        case 'Time'
            readstr = [readstr '%f '];
        otherwise  %get all the markers and marker-valid indicators
            readstr = [readstr '%f '];
    end
end

temp = fgetl(fid); %get the first line of data in the file
tmpline = textscan(temp,readstr);

while(isempty(tmpline{1}) || tmpline{1} ~= 1)
    temp = fgetl(fid); %throw out the any garbage until we hit the first line of real data
    if ~isempty(temp)
        tmpline = textscan(temp,readstr);
    else
        tmpline{1} = [];
    end
end
% for b = 1:length(Dheader.cols)
%     data.(Dheader.cols{b}) = tmpline{b};
%     %eval(sprintf('data.%s = dvals{%d}(%d:%d:end);',a,char(Eheader.cols{b}),b,a,numCODAs));
% end
%dvals = [];
dvals = NaN*ones(Dheader.NumSamples,length(Dheader.cols));

dvals(1,:) = cell2mat(tmpline);
ind = 1;

while ~feof(fid)
    %we have to do this line by line because there may be read "errors" or
    % other comments in the data file so pulling in the entire data file at
    % once could throw an error 
    
    %check if the data is of the right format if so, parse the data and
    % save it into the appropriate arrays in the structure 
    
    %read the next line in the data file
    temp = fgetl(fid);
    
    if ~isempty(temp)
        tmpline = textscan(temp,readstr);
    else
        tmpline{1} = [];
    end
    
    if ~isempty(tmpline{1})
        ind = ind+1;
        dvals(ind,:) = cell2mat(tmpline);
    end
    
end

fclose(fid);

for b = 1:length(Dheader.cols)
    data.(Dheader.cols{b}) = dvals(1:end-1,b);
end

%regroup data by sensor
dat = data;
clear data;

for a = 1:length(Dheader.cols)
    i1 = strfind(Dheader.cols{a},'M');
    i2 = strfind(Dheader.cols{a},'X');
    i3 = strfind(Dheader.cols{a},'Y');
    i4 = strfind(Dheader.cols{a},'Z');
    i5 = strfind(Dheader.cols{a},'Valid');
    
    if ~isempty(i1) && ~isempty(i2)
        snum = str2num(Dheader.cols{a}(i1+1:i2-1));
        data(snum).X = dat.(Dheader.cols{a});
    elseif ~isempty(i1) && ~isempty(i3)
        snum = str2num(Dheader.cols{a}(i1+1:i3-1));
        data(snum).Y = dat.(Dheader.cols{a});
    elseif ~isempty(i1) && ~isempty(i4)
        snum = str2num(Dheader.cols{a}(i1+1:i4-1));
        data(snum).Z = dat.(Dheader.cols{a});
    elseif ~isempty(i1) && ~isempty(i5)
        snum = str2num(Dheader.cols{a}(i1+1:i5-1));
        data(snum).Valid = dat.(Dheader.cols{a});
    end
end

for a = 1:length(Dheader.cols)
    i1 = strfind(Dheader.cols{a},'M');
    i2 = strfind(Dheader.cols{a},'X');
    i3 = strfind(Dheader.cols{a},'Y');
    i4 = strfind(Dheader.cols{a},'Z');
    i5 = strfind(Dheader.cols{a},'Valid');
    
    if isempty(i1) && isempty(i2) && isempty(i3) && isempty(i4) && isempty(i5)
        for b = 1:length(data)
            data(b).(Dheader.cols{a}) = dat.(Dheader.cols{a});
        end
    end
end

clear dvals;


%% merge the event and data file by adding extrapolated columns to the data file

for a = 1:length(Eheader.cols)
    if strcmp(Eheader.cols{a},'Device_Num') %|| strcmp(Eheader.cols{a},'HandX') || strcmp(Eheader.cols{a},'HandY')
        continue;
    end
    
    for b = 1:length(data)
        tmp = NaN*ones(size(data(b).Time));
        
        for c = 1:length(event(b).TrackerTime)
            if c == 1
                ind1 = 1;
                ind2 = find(data(b).Time == event(b).TrackerTime(c));
                tmp(ind1:ind2-1) = 0;
                
                ind1 = find(data(b).Time == event(b).TrackerTime(c));
                ind2 = find(data(b).Time == event(b).TrackerTime(c+1));
                tmp(ind1:ind2-1) = event(b).(Eheader.cols{a})(c);
            elseif c == length(event(b).TrackerTime)
                ind1 = find(data(b).Time == event(b).TrackerTime(c));
                ind2 = length(data(b).Time);
                tmp(ind1:ind2-1) = 0;
            else
                ind1 = find(data(b).Time == event(b).TrackerTime(c));
                ind2 = find(data(b).Time == event(b).TrackerTime(c+1));
                tmp(ind1:ind2-1) = event(b).(Eheader.cols{a})(c);
            end
        end
        
        data(b).(Eheader.cols{a}) = tmp;
    end
end



%%



if nargout > 2
    varargout{1} = fname;
end
if nargout > 3
    varargout{2} = fpath;
end
if nargout > 4
    varargout{3} = Eheader;
end
if nargout > 5
    varargout{3} = event;
end



