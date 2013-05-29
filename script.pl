#!/usr/bin/perl
use Path::Class;

#   Motion Console Script
#
#   Author: Oliver Wilkie
#
#   This is a script for running batch processing on a video testing all possible parameters
#   It is worth ensuring the $output_dir exists prior to running the script
#   Each result is stored in a seperate sub-folder and contains a text file with the command
#     that was used to generate the output.
#   It must be run from the same directory as the MotionConsole executable
#


# Options
$input_video = '/homes/osw09/Motion/Examples/Heart/one-channels-short.mov';
$output_dir = '/vol/bitbucket/osw09/HeartResults/';
@cropCornerX = (35,26,10);
@cropCornerY = (135,109,62);
@cropWidth = (88,133,285);
@cropHeight = (80,121,171);
@featureDetectionMethods = ('goodtt','goodtth','sift','surf','fast');
@featureWindowSizes = (0,50,100,200);
$manual_dir = '/homes/osw09/Motion/Examples/Heart/HeartManualMarkings';
@salientTracking = (0,1);
@gravitateToCenter = (0,1);


# Main Program
mkdir($output_dir);
$count = 1;
foreach $cropIndex (0..$#cropCornerX) {
  $cropCorner = $cropCornerX[$cropIndex] . ',' . $cropCornerY[$cropIndex];
  $cropSize = $cropWidth[$cropIndex] . ',' . $cropHeight[$cropIndex];
  foreach $fdmethod (@featureDetectionMethods) {
    foreach $fdwindow (@featureWindowSizes) {
      # Do it with Salient Turned off
      $dir = $output_dir . $count;
      mkdir($dir);
      $dirHandle = dir($dir);
      $file = $dirHandle->file("info.txt");
      $file_handle = $file->openw();
      $command = "./MotionConsole -I $input_video -O $dir/output.avi -C $cropCorner -S $cropSize -F $fdmethod -W $fdwindow -D";
      $file_handle->print($command);
      $rc = system($command);
      if ($rc & 127) { die "signal death" } 
      $count += 1;
      # Do it with Salient Turned on
      foreach $gravitate ($gravitateToCenter) {
        $dir = $output_dir . $count;
        mkdir($dir);
        $dirHandle = dir($dir);
        $file = $dirHandle->file("info.txt");
        $file_handle = $file->openw();
        $command = "./MotionConsole -I $input_video -O $dir/output.avi -C $cropCorner -S $cropSize -F $fdmethod -W $fdwindow --salient-path-tracking -M $manual_dir -G $gravitate -D";
        $file_handle->print($command);
        $rc = system($command);
        if ($rc & 127) { die "signal death" }   
        $count += 1;
      }
    }
  }
}







mkdir($output_dir);

