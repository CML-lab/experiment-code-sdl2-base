%Matlab code for use with calibration of the kinereach hardware.
%  This code assumes that you have already done the following:
%    1. Remove the half-silvered mirror so there is nothing between the
%        TV monitor and the tabletop where the kinereach sensors will be
%        located.
%    2. Run the kinereach C++ code and have it display the calibration
%        trace ('Trace0.png').
%    3. Use a plumb bob to position the FOB tracker directly in line with
%        each of the calibration crosses on the screen while recording data
%        into a data file.
%    4. Modify the hard-coded path and filename below, and run the code.
%    5. Mark the data points as instructed.
%    6. In the code, hard-code the calibration values reported.

clear all;
close all;

%fpath = 'C:\Users\Aaron\Documents\kinereach\KinereachSoftware\calibration\';
%fname = 'calibration_good.txt';

%do positions first
[data,params,fname,fpath] = KRload();

figure(1)
plot(data{1}.HandX,data{1}.HandY,'b');
hold on;
plot(data{3}.HandX,data{3}.HandY,'r');
hold off;

disp('Mark positions in the data file where the flock of birds sensor is aligned at each calibration target.');
disp('  Use the left mouse click to mark a point, and the right mouse click to remove a point.');
disp('  Use the ''n'' key to advance in the data file, and the ''b'' key to go to the preceeding portion of ');
disp('    the data set.  Hit the ''x'' key when all positions have been marked, to quit.');
disp('  Calibration will be performed separately for the right and left hand FOB trackers.');

doloop = 1;
while doloop
    figure(2)
    rhind = markdata4(data{1}.HandX,data{1}.HandY,data{3}.HandX,data{3}.HandY,10000,0,[],0,130,'Name','Mark Right Hand Regions');
    
    figure(1)
    plot(data{1}.HandX,data{1}.HandY,'b');
    hold on;
    plot(data{3}.HandX,data{3}.HandY,'r');
    plot(data{3}.HandX(rhind),data{3}.HandY(rhind),'ko');
    hold off;
    
    doloop = input('Redo marks? (1 = Repeat, 0 = Stop): ');
end

doloop = 1;
while doloop
    figure(2)
    lhind = markdata4(data{3}.HandX,data{3}.HandY,data{1}.HandX,data{1}.HandY,10000,0,[],0,130,'Name','Mark Left Hand Regions','indx',rhind,'indy',rhind);
    
    figure(1)
    plot(data{1}.HandX,data{1}.HandY,'b');
    hold on;
    plot(data{1}.HandX(lhind),data{1}.HandY(lhind),'co');
    plot(data{3}.HandX,data{3}.HandY,'r');
    plot(data{3}.HandX(rhind),data{3}.HandY(rhind),'kx');
    hold off;
    
    doloop = input('Redo marks? (1 = Repeat, 0 = Stop): ');
end


%calculate translation and rotation matrices
physical_width = 1.21;
physical_height = 0.68;
screen_width = 1920;
screen_height = 1080;
pix2deg = physical_width/screen_width;


% display.x = [screen_width/8     7*screen_width/8
%              screen_width/8     7*screen_width/8
%              screen_width/8     7*screen_width/8
%             ];
% 
% display.y = [7*screen_height/8  7*screen_height/8
%                screen_height/2    screen_height/2
%                screen_height/8    screen_height/8
%             ];
% display.z = [0 0
%              0 0
%              0 0
%             ];
display.x = [screen_width/8     screen_width/2     7*screen_width/8
             screen_width/8     screen_width/2     7*screen_width/8
             screen_width/8     screen_width/2     7*screen_width/8
            ];

display.y = [7*screen_height/8   7*screen_height/8   7*screen_height/8
               screen_height/2     screen_height/2     screen_height/2
               screen_height/8     screen_height/8     screen_height/8
            ];
display.z = [0 0 0
             0 0 0
             0 0 0
            ];
              

display.x = display.x*pix2deg;
display.y = display.y*pix2deg;
              
hand{1}.hand = 'left';
hand{1}.hand = 'right';


% hand{1}.x = [mean(data{1}.HandX(lhind(5):lhind(6)))    mean(data{1}.HandX(lhind(7):lhind(8)))
%              mean(data{1}.HandX(lhind(3):lhind(4)))    mean(data{1}.HandX(lhind(9):lhind(10)))
%              mean(data{1}.HandX(lhind(1):lhind(2)))    mean(data{1}.HandX(lhind(11):lhind(12)))
%             ];
% hand{1}.y = [mean(data{1}.HandY(lhind(5):lhind(6)))    mean(data{1}.HandY(lhind(7):lhind(8)))
%              mean(data{1}.HandY(lhind(3):lhind(4)))    mean(data{1}.HandY(lhind(9):lhind(10)))
%              mean(data{1}.HandY(lhind(1):lhind(2)))    mean(data{1}.HandY(lhind(11):lhind(12)))
%             ];
% hand{1}.z = [mean(data{1}.HandZ(lhind(5):lhind(6)))    mean(data{1}.HandZ(lhind(7):lhind(8)))
%              mean(data{1}.HandZ(lhind(3):lhind(4)))    mean(data{1}.HandZ(lhind(9):lhind(10)))
%              mean(data{1}.HandZ(lhind(1):lhind(2)))    mean(data{1}.HandZ(lhind(11):lhind(12)))
%             ];
% 
%         
% hand{2}.x = [mean(data{3}.HandX(rhind(5):rhind(6)))    mean(data{3}.HandX(rhind(7):rhind(8)))
%              mean(data{3}.HandX(rhind(5):rhind(6)))    mean(data{3}.HandX(rhind(9):rhind(10)))
%              mean(data{3}.HandX(rhind(1):rhind(2)))    mean(data{3}.HandX(rhind(11):rhind(12)))
%             ];
% hand{2}.y = [mean(data{3}.HandY(rhind(5):rhind(6)))    mean(data{3}.HandY(rhind(7):rhind(8)))
%              mean(data{3}.HandY(rhind(3):rhind(4)))    mean(data{3}.HandY(rhind(9):rhind(10)))
%              mean(data{3}.HandY(rhind(1):rhind(2)))    mean(data{3}.HandY(rhind(11):rhind(12)))
%             ];
% hand{2}.z = [mean(data{3}.HandZ(rhind(5):rhind(6)))    mean(data{3}.HandZ(rhind(7):rhind(8)))
%              mean(data{3}.HandZ(rhind(3):rhind(4)))    mean(data{3}.HandZ(rhind(9):rhind(10)))
%              mean(data{3}.HandZ(rhind(1):rhind(2)))    mean(data{3}.HandZ(rhind(11):rhind(12)))
%             ];
hand{1}.x = [mean(data{1}.HandX(lhind(1):lhind(2)))    mean(data{1}.HandX(lhind(3):lhind(4)))    mean(data{1}.HandX(lhind(5):lhind(6)))
             mean(data{1}.HandX(lhind(11):lhind(12)))  mean(data{1}.HandX(lhind(9):lhind(10)))   mean(data{1}.HandX(lhind(7):lhind(8)))
             mean(data{1}.HandX(lhind(13):lhind(14)))  mean(data{1}.HandX(lhind(15):lhind(16)))  mean(data{1}.HandX(lhind(17):lhind(18)))
            ];
hand{1}.y = [mean(data{1}.HandY(lhind(1):lhind(2)))    mean(data{1}.HandY(lhind(3):lhind(4)))    mean(data{1}.HandY(lhind(5):lhind(6)))
             mean(data{1}.HandY(lhind(11):lhind(12)))  mean(data{1}.HandY(lhind(9):lhind(10)))   mean(data{1}.HandY(lhind(7):lhind(8)))
             mean(data{1}.HandY(lhind(13):lhind(14)))  mean(data{1}.HandY(lhind(15):lhind(16)))  mean(data{1}.HandY(lhind(17):lhind(18)))
            ];
hand{1}.z = [mean(data{1}.HandZ(lhind(1):lhind(2)))    mean(data{1}.HandZ(lhind(3):lhind(4)))    mean(data{1}.HandZ(lhind(5):lhind(6)))
             mean(data{1}.HandZ(lhind(11):lhind(12)))  mean(data{1}.HandZ(lhind(9):lhind(10)))   mean(data{1}.HandZ(lhind(7):lhind(8)))
             mean(data{1}.HandZ(lhind(13):lhind(14)))  mean(data{1}.HandZ(lhind(15):lhind(16)))  mean(data{1}.HandZ(lhind(17):lhind(18)))
            ];
        
hand{2}.x = [mean(data{3}.HandX(rhind(1):rhind(2)))    mean(data{3}.HandX(rhind(3):rhind(4)))    mean(data{3}.HandX(rhind(5):rhind(6)))
             mean(data{3}.HandX(rhind(11):rhind(12)))  mean(data{3}.HandX(rhind(9):rhind(10)))   mean(data{3}.HandX(rhind(7):rhind(8)))
             mean(data{3}.HandX(rhind(13):rhind(14)))  mean(data{3}.HandX(rhind(15):rhind(16)))  mean(data{3}.HandX(rhind(17):rhind(18)))
            ];
hand{2}.y = [mean(data{3}.HandY(rhind(1):rhind(2)))    mean(data{3}.HandY(rhind(3):rhind(4)))    mean(data{3}.HandY(rhind(5):rhind(6)))
             mean(data{3}.HandY(rhind(11):rhind(12)))  mean(data{3}.HandY(rhind(9):rhind(10)))   mean(data{3}.HandY(rhind(7):rhind(8)))
             mean(data{3}.HandY(rhind(13):rhind(14)))  mean(data{3}.HandY(rhind(15):rhind(16)))  mean(data{3}.HandY(rhind(17):rhind(18)))
            ];
hand{2}.z = [mean(data{3}.HandZ(rhind(1):rhind(2)))    mean(data{3}.HandZ(rhind(3):rhind(4)))    mean(data{3}.HandZ(rhind(5):rhind(6)))
             mean(data{3}.HandZ(rhind(11):rhind(12)))  mean(data{3}.HandZ(rhind(9):rhind(10)))   mean(data{3}.HandZ(rhind(7):rhind(8)))
             mean(data{3}.HandZ(rhind(13):rhind(14)))  mean(data{3}.HandZ(rhind(15):rhind(16)))  mean(data{3}.HandZ(rhind(17):rhind(18)))
            ];

hand{1}.offsetX = mean(reshape(display.x-hand{1}.x,[],1));
hand{1}.offsetY = mean(reshape(display.y-hand{1}.y,[],1));
hand{1}.offsetZ = mean(reshape(display.z-hand{1}.z,[],1));

hand{2}.offsetX = mean(reshape(display.x-hand{2}.x,[],1));
hand{2}.offsetY = mean(reshape(display.y-hand{2}.y,[],1));
hand{2}.offsetZ = mean(reshape(display.z-hand{2}.z,[],1));

%the true offset is the average of both hands, since the two hands were
%placed on either side of the plumb bob
offsetX = mean([hand{1}.offsetX hand{2}.offsetX]);
offsetY = mean([hand{1}.offsetY hand{2}.offsetY]);
offsetZ = mean([hand{1}.offsetZ hand{2}.offsetZ]);

hand{1}.trueOffsetX = offsetX;
hand{2}.trueOffsetX = offsetX;
hand{1}.trueOffsetY = offsetY;
hand{2}.trueOffsetY = offsetY;
hand{1}.trueOffsetZ = offsetZ;
hand{2}.trueOffsetZ = offsetZ;


%calculate rotation angle
display.centerX = physical_width/2;
display.centerY = physical_height/2;

hand{1}.centeredX = hand{1}.x+offsetX-display.centerX;
hand{2}.centeredX = hand{2}.x+offsetX-display.centerX;
hand{1}.centeredY = hand{1}.y+offsetY-display.centerY;
hand{2}.centeredY = hand{2}.y+offsetY-display.centerY;

display.centeredX = display.x-display.centerX;
display.centeredY = display.y-display.centerY;

%hand{1}.angles = acos( (hand{1}.centeredX.*display.centeredX + hand{1}.centeredY.*display.centeredY)/(sqrt(hand{1}.centeredX.^2+hand{1}.centeredY.^2).*sqrt(display.centeredX.^2+display.centeredY.^2)) );
hand{1}.angles = atan2(hand{1}.centeredY,hand{1}.centeredX)-atan2(display.centeredY,display.centeredX); 
hand{1}.angles(hand{1}.angles>pi) = hand{1}.angles(hand{1}.angles>pi) - 2*pi;
hand{1}.angles(hand{1}.angles<-pi) = hand{1}.angles(hand{1}.angles<-pi) + 2*pi;
hand{1}.meanrotang = mean(reshape(hand{1}.angles,[],1));

hand{2}.angles = atan2(hand{2}.centeredY,hand{2}.centeredX)-atan2(display.centeredY,display.centeredX); 
hand{2}.angles(hand{2}.angles>pi) = hand{2}.angles(hand{2}.angles>pi) - 2*pi;
hand{2}.angles(hand{2}.angles<-pi) = hand{2}.angles(hand{2}.angles<-pi) + 2*pi;
hand{2}.meanrotang = mean(reshape(hand{1}.angles,[],1));

rotmat{1} = [cos(-hand{1}.meanrotang) -sin(-hand{1}.meanrotang)
             sin(-hand{1}.meanrotang)  cos(-hand{1}.meanrotang)
            ];
rotmat{2} = [cos(-hand{2}.meanrotang) -sin(-hand{2}.meanrotang)
             sin(-hand{2}.meanrotang)  cos(-hand{2}.meanrotang)
            ];

for c = 1:2
    for a = 1:size(hand{1}.centeredX,1)
        for b = 1:size(hand{1}.centeredX,2)
            temp = rotmat{c}*[hand{c}.centeredX(a,b);hand{c}.centeredY(a,b)];
            hand{c}.rotX(a,b) = temp(1) + display.centerX;
            hand{c}.rotY(a,b) = temp(2) + display.centerY;
        end
    end
end

fprintf('\n\n');
disp('Save the following values in the config.h file in your code: ');
fprintf('X Offset: %.5f meters\n',offsetX);
fprintf('Y Offset: %.5f meters\n',offsetY);
fprintf('Rotation angle: %.5f radians\n\n',mean([-hand{1}.meanrotang -hand{2}.meanrotang]));

figure
plot(display.x,display.y,'ko');
hold on;
plot(hand{1}.rotX,hand{1}.rotY,'b+');
plot(hand{2}.rotX,hand{2}.rotY,'rx');

clear a b c doloop offset* rotmat 

save([fpath fname(1:strfind(fname,'.')-1)]);