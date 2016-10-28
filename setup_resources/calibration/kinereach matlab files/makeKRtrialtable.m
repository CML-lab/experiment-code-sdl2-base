%make kinereach trial tables, new software version (c++ code)

clear all;

doseqlearn2tgt4 = 0;
doseqlearn2tgt8 = 1;




if doseqlearn2tgt4 == 1
    
    rng('shuffle');
    
    SeqLength = 4;
    NTgt = 4;
    

    NTgtTrain = NTgt-1;
%     tgtorders = [[1 4 3 2];
%                  [2 1 4 3];
%                  [3 2 1 4];
%                  [4 3 2 1];
%                 ];  %clockwise
    tgtorders = [[1 2 3 4];
                 [2 3 4 1];
                 [3 4 1 2];
                 [4 1 2 3];
                ];  %counterclockwise


    NTrainBlocks = 5;
    NRandBlocks = 4;
    
    %timing parameters
    ITI = 1000;     %inter-trial time, within sequence
    %ISImin = 2000;  %inter-sequence time minimum
    %ISImax = 4000;  %inter-sequence time maximum
    %randi([ISImin,ISImax],1)
    
    %target parameters
    physical_width = 121; %centimeters;
    screen_width = 1366;
    pix2meters = screen_width/physical_width;
    starttgt = [121/2 24];
    tgts = [ 10   0;
              0  10;
            -10   0;
              0 -10;
           ];  %relative to start target, in cm
    starttgt = starttgt/100;    %convert to meters;
    tgts = tgts/100+repmat(starttgt,size(tgts,1),1);  %write tgts in absolute coordinates
    

    NSeqBlock = 1+(NTgtTrain)*6;  %each of the NTgt-1 training positions six times, and 1 probe
    Ntrials = (SeqLength)*NSeqBlock;

    %trainPos = randperm(NTgt);    %the last 1-2 positions will be the "probe" positions
    trainPos = [randperm(3) 4];

    for ntb = 1:NTrainBlocks
        
        fid = fopen(sprintf('s2t4train%d.txt',ntb),'wt');
        
        testorder = [];
        for a = 1:(NSeqBlock-1)/(NTgtTrain)   %number of times testing at each position per block
            tmp = randperm(NTgtTrain);
            if isempty(testorder)
                testorder = tmp;
            else
                while (tmp(1) == testorder(end))
                    tmp = randperm(NTgtTrain);
                end
                testorder = [testorder tmp];
            end
        end
                
        testorder(1,:) = trainPos(testorder(1,:));
        testorder = [testorder trainPos(end-(round((NTgt-NTgtTrain-1)*rand)))];  %select one of the non-train positions for the probe

        for a = 1:NSeqBlock
            %end make trorder array
            trorder = tgtorders(testorder(a),:);
            
            %print out the trial table
            for b = 1:length(trorder)
                if b == 1
                    fprintf(fid,'1 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,trorder(b));
                else
                    fprintf(fid,'0 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,9);
                end
            end
            
            clear trorder;
            
        end
        
        fclose(fid);
    end
    
    
    for ntb = 1:NRandBlocks
        
        fid = fopen(sprintf('s2t4rand%d.txt',ntb),'wt');
        
        testorder = [];
        testorder = [];
        for a = 1:(NSeqBlock-1)/(NTgtTrain)   %number of times testing at each position per block
            tmp = randperm(NTgtTrain);
            if isempty(testorder)
                testorder = tmp;
            else
                while (tmp(1) == testorder(end))
                    tmp = randperm(NTgtTrain);
                end
                testorder = [testorder tmp];
            end
        end
                
        testorder(1,:) = trainPos(testorder(1,:));
        testorder = [testorder trainPos(end-(round((NTgt-NTgtTrain-1)*rand)))];  %select one of the non-train positions for the probe

        for a = 1:NSeqBlock
            trorder = randi(NTgt,1,NTgt);
            trorder = [testorder(1,a) trorder(1:3)];
            while ~isempty(find(all(repmat(trorder,size(tgtorders,1),1)==tgtorders,2))) || length(unique(trorder))<=2 || any(diff(trorder)==0) %prevent rand condition from displaying a training condition
                trorder = randi(NTgt,1,NTgt);
                trorder = [testorder(1,a) trorder(1:3)];
            end  %end make trorder array
            
            %print out the trial table
            for b = 1:length(trorder)
                if b == 1
                    fprintf(fid,'1 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,trorder(b)+NTgt);
                else
                    fprintf(fid,'0 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,10);
                end
            end
            
            clear trorder;
            
        end
        fclose(fid);
    end
                
end




if doseqlearn2tgt8 == 1
    
    rng('shuffle');
    
    SeqLength = 5;
    NTgt = 4;   %targets to prompt for initial reaches
    

    NTgtTrain = NTgt-1;
    tgtorders = [[2 7 8 5 4];
                 [4 1 2 7 6];
                 [6 3 4 1 8];
                 [8 5 6 3 2];
                ];  %counterclockwise


    NTrainBlocks = 5;
    NRandBlocks = 4;
    
    %timing parameters
    ITI = 1000;     %inter-trial time, within sequence
    %ISImin = 2000;  %inter-sequence time minimum
    %ISImax = 4000;  %inter-sequence time maximum
    %randi([ISImin,ISImax],1)
    
    %target parameters
    physical_width = 121; %centimeters;
    screen_width = 1280;
    pix2meters = screen_width/physical_width;
    starttgt = [physical_width/2 18];
    tgts = [ 10.00    0.00;
              7.07    7.07;
              0.00   10.00;
             -7.07    7.07;
            -10.00    0.00;
             -7.07   -7.07;
              0.00  -10.00;
              7.07   -7.07;
           ];  %relative to start target, in cm
    starttgt = starttgt/100;    %convert to meters;
    tgts = tgts/100+repmat(starttgt,size(tgts,1),1);  %write tgts in absolute coordinates
    

    NSeqBlock = 1+(NTgtTrain)*7;  %each of the NTgt-1 training positions seven times, and 1 probe
    Ntrials = (SeqLength)*NSeqBlock;

    %trainPos = randperm(NTgt);    %the last position will be the "probe" positions
    trainPos = [randperm(3)];
    trainPos(find(trainPos==3)) = 4;
    trainPos = [trainPos 3];

    for ntb = 1:NTrainBlocks
        
        fid = fopen(sprintf('s2t8train%d.txt',ntb),'wt');
        
        testorder = [];
        for a = 1:(NSeqBlock-1)/(NTgtTrain)   %number of times testing at each position per block
            tmp = randperm(NTgtTrain);
            if isempty(testorder)
                testorder = tmp;
            else
                while (tmp(1) == testorder(end))
                    tmp = randperm(NTgtTrain);
                end
                testorder = [testorder tmp];
            end
        end
                
        testorder(1,:) = trainPos(testorder(1,:));
        testorder = [testorder trainPos(end-(round((NTgt-NTgtTrain-1)*rand)))];  %select one of the non-train positions for the probe

        for a = 1:NSeqBlock
            %end make trorder array
            trorder = tgtorders(testorder(a),:);
            
            %print out the trial table
            for b = 1:length(trorder)
                if b == 1
                    fprintf(fid,'1 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,(trorder(b))/2);
                else
                    fprintf(fid,'0 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,9);
                end
            end
            
            clear trorder;
            
        end
        
        fclose(fid);
    end
    
    
    for ntb = 1:NRandBlocks
        
        fid = fopen(sprintf('s2t8rand%d.txt',ntb),'wt');
        
        testorder = [];
        testorder = [];
        for a = 1:(NSeqBlock-1)/(NTgtTrain)   %number of times testing at each position per block
            tmp = randperm(NTgtTrain);
            if isempty(testorder)
                testorder = tmp;
            else
                while (tmp(1) == testorder(end))
                    tmp = randperm(NTgtTrain);
                end
                testorder = [testorder tmp];
            end
        end
                
        testorder(1,:) = trainPos(testorder(1,:));
        testorder = [testorder trainPos(end-(round((NTgt-NTgtTrain-1)*rand)))];  %select one of the non-train positions for the probe
        
        %make array of all possible reaches that will occur during one
        %training block
        alltgtreaches = [];
        for a = 1:length(testorder)
            alltgtreaches = [alltgtreaches tgtorders(testorder(a),2:end)];
        end
        ind = randperm(length(alltgtreaches));
        alltgtreaches = alltgtreaches(ind);
        temp = [tgtorders(testorder,1) reshape(alltgtreaches,[],4)];
        while any(any(diff(temp,1,2)==0,2))
            ind = randperm(length(alltgtreaches));
            alltgtreaches = alltgtreaches(ind);
            temp = [tgtorders(testorder,1) reshape(alltgtreaches,[],4)];
        end
        for a = 1:size(temp,1)
            while any(all(repmat(temp(a,:),size(tgtorders,1),1)==tgtorders,2))
                temp(a,2:end) = temp(a,randperm(4)+1);
            end
        end
        alltgtreaches = temp; %this now contains one row vector for each sequence in the rand block!
        
        for a = 1:NSeqBlock
            trorder = alltgtreaches(a,:);
            
            %print out the trial table
            for b = 1:length(trorder)
                if b == 1
                    fprintf(fid,'1 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,(trorder(b))/2+NTgt);
                else
                    fprintf(fid,'0 %.3f %.3f %.3f %.3f %d %d\n', starttgt(1),starttgt(2),tgts(trorder(b),1),tgts(trorder(b),2), ITI,10);
                end
            end
            
            clear trorder;
            
        end
        fclose(fid);
    end
                
end