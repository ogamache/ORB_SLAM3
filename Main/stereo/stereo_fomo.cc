/**
 * This file is part of ORB-SLAM3
 *
 * Copyright (C) 2017-2021 Carlos Campos, Richard Elvira, Juan J. Gómez Rodríguez, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
 * Copyright (C) 2014-2016 Raúl Mur-Artal, José M.M. Montiel and Juan D. Tardós, University of Zaragoza.
 *
 * ORB-SLAM3 is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ORB-SLAM3 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
 * the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with ORB-SLAM3.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <stdio.h>
#include <dirent.h>

#include <opencv2/core/core.hpp>

#include <System.h>

using namespace std;

void LoadImages(const string &strPathLeft, const string &strPathRight, vector<string> &vstrImageLeft,
                vector<string> &vstrImageRight, vector<double> &vTimeStamps);

int main(int argc, char **argv)
{
    if (argc != 6)
    {
        cerr << endl
                << "Usage: ./fomo_stereo path_to_vocabulary path_to_settings path_to_left_folder path_to_right_folder save_path" << endl;
        return 1;
    }

    // Retrieve paths to images
    vector<string> vstrImageLeft;
    vector<string> vstrImageRight;
    vector<double> vTimeStamp;
    LoadImages(string(argv[3]), string(argv[4]), vstrImageLeft, vstrImageRight, vTimeStamp);
    std::string savePath = argv[5];
    // Create save folder
    if (opendir(savePath.c_str()) != nullptr)
    {
        std::string command_remove = "rm -rf " + savePath;
        system(command_remove.c_str());
    }

    if (vstrImageLeft.empty() || vstrImageRight.empty())
    {
        cerr << "ERROR: No images in provided path." << endl;
        return 1;
    }

    if (vstrImageLeft.size() != vstrImageRight.size())
    {
        cerr << "ERROR: Different number of left and right images." << endl;
        return 1;
    }

    const int nImages = vstrImageLeft.size();

    // Create SLAM system. It initializes all system threads and gets ready to process frames.
    ORB_SLAM3::System SLAM(argv[1],argv[2],ORB_SLAM3::System::STEREO,true);

    // Vector for tracking time statistics
    vector<float> vTimesTrack;
    vTimesTrack.resize(nImages);

    cout << endl << "-------" << endl;
    cout << "Start processing sequence ..." << endl;
    cout << "Images in the sequence: " << nImages << endl << endl;   

    double t_track = 0.f;
    double t_resize = 0.f;

    cv::Mat imLeft, imRight;
    for(int ni=0; ni<nImages; ni++)
    {
        // Read left and right images from file
        imLeft = cv::imread(vstrImageLeft[ni], cv::IMREAD_UNCHANGED);   //,cv::IMREAD_UNCHANGED);
        imRight = cv::imread(vstrImageRight[ni], cv::IMREAD_UNCHANGED); //,cv::IMREAD_UNCHANGED);
        double tframe = vTimeStamp[ni];

        if (imLeft.empty())
        {
            cerr << endl
                    << "Failed to load image at: "
                    << string(vstrImageLeft[ni]) << endl;
            return 1;
        }

        if (imRight.empty())
        {
            cerr << endl
                    << "Failed to load image at: "
                    << string(vstrImageRight[ni]) << endl;
            return 1;
        }


#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t1 = std::chrono::monotonic_clock::now();
#endif

        // Pass the images to the SLAM system
        SLAM.TrackStereo(imLeft, imRight, tframe, vector<ORB_SLAM3::IMU::Point>(), vstrImageLeft[ni]);

#ifdef COMPILEDWITHC11
        std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
#else
        std::chrono::monotonic_clock::time_point t2 = std::chrono::monotonic_clock::now();
#endif

#ifdef REGISTER_TIMES
        t_track = t_resize + t_rect + std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(t2 - t1).count();
        SLAM.InsertTrackTime(t_track);
#endif

        double ttrack = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count();

        vTimesTrack[ni] = ttrack;

        // Wait to load the next frame
        double T=0;
        if(ni<nImages-1)
            T = vTimeStamp[ni+1]-tframe;
        else if(ni>0)
            T = tframe-vTimeStamp[ni-1];

        if(ttrack<T)
            usleep((T-ttrack)*1e6);
    }

    // Stop all threads
    SLAM.Shutdown();

    // Tracking time statistics
    sort(vTimesTrack.begin(),vTimesTrack.end());
    float totaltime = 0;
    for(int ni=0; ni<nImages; ni++)
    {
        totaltime+=vTimesTrack[ni];
    }
    cout << "-------" << endl << endl;
    cout << "median tracking time: " << vTimesTrack[nImages/2] << endl;
    cout << "mean tracking time: " << totaltime/nImages << endl;

    // Save camera trajectory
    // SLAM.SaveTrajectoryKITTI("CameraTrajectory.txt");
    SLAM.SaveTrajectoryEuRoC(savePath + "/StereoCameraTrajectory.txt");

    return 0;
}

void LoadImages(const string &strPathLeft, const string &strPathRight, vector<string> &vstrImageLeft,
                vector<string> &vstrImageRight, vector<double> &vTimeStamps)
{
    struct dirent *entry = nullptr;
    DIR *dp = nullptr;

    dp = opendir(strPathLeft.c_str());
    if (dp != nullptr)
    {
        std::vector<std::string> filenames;
        while ((entry = readdir(dp)))
        {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            filenames.push_back(entry->d_name);
        }
        std::sort(filenames.begin(), filenames.end());

        for (const auto &filename : filenames)
        {
            // std::string filename = entry->d_name;
            std::string timestampStr = filename.substr(0, filename.find_last_of('.'));
            vstrImageLeft.push_back(strPathLeft + "/" + filename);
            vTimeStamps.push_back(std::stod(timestampStr) / 1e9);
        }
    }
    else
    {
        perror("Couldn't open the directory");
    }

    // Try to match the left and right images
    std::vector<int> toRemove;
    for (int i = 0; i < vstrImageLeft.size(); i++)
    {
        // Check if the right image exists
        std::string imgName = vstrImageLeft[i].substr(vstrImageLeft[i].find_last_of('/') + 1);
        std::ifstream f(strPathRight + "/" + imgName);
        if (f.good())
        {
            vstrImageRight.push_back(strPathRight + "/" + imgName);
        }
        else
        {
            toRemove.push_back(i);
        }
    }

    // Remove the images that don't have a corresponding right image
    for (int i = toRemove.size() - 1; i >= 0; i--)
    {
        vstrImageLeft.erase(vstrImageLeft.begin() + toRemove[i]);
        vTimeStamps.erase(vTimeStamps.begin() + toRemove[i]);
    }

    closedir(dp);
}
