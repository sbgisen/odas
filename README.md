ODAS ![Analytics](https://ga-beacon.appspot.com/UA-27707792-4/github-main?pixel) 
=======

ODAS stands for Open embeddeD Audition System. This is a library dedicated to perform sound source localization, tracking, separation and post-filtering. ODAS is coded entirely in C, for more portability, and is optimized to run easily on low-cost embedded hardware. ODAS is free and open source.

The [ODAS wiki](https://github.com/introlab/odas/wiki) describes how to build and run the software. 

[![ODAS Demonstration](https://img.youtube.com/vi/n7y2rLAnd5I/0.jpg)](https://youtu.be/n7y2rLAnd5I)

# Graphical User Interface (GUI) for Data Visualization

Please have a look at the [odas_web](https://github.com/introlab/odas_web) project.
![GUI](https://github.com/introlab/odas_web/blob/master/screenshots/live_data.png)


# Open Source Hardware from IntRoLab

* [8SoundsUSB](https://sourceforge.net/projects/eightsoundsusb/), 8 inputs, USB powered, configurable microphone array.
* [16SoundsUSB](https://github.com/introlab/16SoundsUSB), 16 inputs, USB powered, configurable microphone array.

# Paper

You can find more information about the methods implemented in ODAS in this paper: 

* F. Grondin and F. Michaud, [Lightweight and Optimized Sound Source Localization and Tracking Methods for Opened and Closed Microphone Array Configurations](https://arxiv.org/pdf/1812.00115), Robotics and Autonomous Systems, 2019 


# Use odas for Azure Kinect DK 

## build odas

```bash
# prerequisites
sudo apt-get install libfftw3-dev libconfig-dev libasound2-dev

# build odas
git clone https://github.com/xiaotaw/odas.git
cd odas && mkdir build && cd build

cmake .. && make
```

## Example 1: Use Azure Kinect DK
```bash
# check Azure kinect DK is connected. 
arecord -l
```

modify config file, according to the Card number and Device number
```bash
vi config/odaslive/azure_kinect_dk.cfg
```

```vi
raw: {
   interface: {
        type = "soundcard";
        # here is the Card number of Azure Kinect DK shown in `arecord -l`
        card = 2;       
        # here is the Device number of Azure Kinect DK shown in `arecord -l`
        device = 0;
    }
}

# let save the output into file 
sss: {
    separated: {
        interface: {
            type = "file";
            path = "separated.raw";
        };        
    };
    postfiltered: {
        interface: {
            type = "file";
            path = "postfiltered.raw";
        };        
    };
}
```


```bash
# run
cd bin
./odaslive -vc ../config/odaslive/azure_kinect_dk.cfg

# the output *.raw files, could be opened by Audacity
```

## Example 2: save output into unix domain socket for IPC
just change config file

```bash
vi config/odaslive/azure_kinect_dk.cfg
```

```vi
sss: {
    separated: {
        interface: {
            type = "file";
            path = "separated.raw";
        };
    };
    postfiltered: {
        interface: {
            type = "unix_domain_socket";
            # change path to your server's socket path
            uds_path = "/data/odas/demo/tools/server.socket";
        };
    };
}
```
and run server
```bash
cd demo/tools

# make sure current dir matches the uds_path
pwd

# build server demo, and run
gcc uds_server.c -o uds_server
./uds_server

```
open another shell
```bash
cd odas/bin
./odaslive -vc ../config/odaslive/azure_kinect_dk.cfg
```

