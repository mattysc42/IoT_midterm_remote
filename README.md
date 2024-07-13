# IoT_midterm_remote
This is a repository for my Deepdive IoT midterm. I'm making a room controller that automatically plays music and controls lights and outlets over a network.

Currently, the track names and artists are manually added in a seperate file. The code is set up to use two ten track playlists. If you want to change the tracks in the playlists, you need to add a three digit number to the front of the track name (001_Track_One_Example.mp3). Then, you need to add the artist and track name to the switch statement in the playlist.h file. 

To power users: 
Feel free to alter the code to allow for more or fewer songs/playlists. 

The mp3s are stored in an SD card inserted in the MP3 module.