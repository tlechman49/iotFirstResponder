# iotFirstResponder
Senior Design Project to create an IoT system that can be used to fight fires

# Quick Start

## Developer setup
Follow the software start up guide located on the google drive.

## Retrieving node ID numbers
1. Connect your computer to a Wi-Fi network
2. Run the GUI and check what the IP Address listed is(That is your computer's IP)
3. In wifi_task.cpp, edit the wifi_task::host_ip variable to match your computer's IP
4. In wifi_task.cpp, edit function wifi_task::wifi_task() such that _ssid and _pwd match your current Wi-Fi credentials
5. In wifi_task.cpp, edit function wifi_task::wifi_task() such that xTaskNotify sends 0x00 if sensors are attached and 0x01 otherwise
6. Upload the firmware to the ECMUs
7. On the GUI initialization window, set it to one node and press start
8. Open the serial terminal on the node and type in "demo" and press enter
9. Record the identifier for the node listed by the python terminal(You should write it on the device or on a piece of tape as well)
10. Repeat recording the identifier for each node (steps 5-8)

## Setting up the network
1. Add a row to nodeButtons.csv for each node ID. (x,y give the location in the GUI)
2. For each output attached to each node, fill in the information in nodeOutputAssignments.csv and doorButtons.csv. The former is for the network and the latter is to create the GUI objects for doors
3. Rerun the GUI but enter the total number of nodes being used and press start
4. Reset all of the nodes
5. Enjoy!

# Troubleshooting
If there is a git problem, give up probably lmao (Or use Github Desktop to troubleshoot: desktop.github.com)
If there is a "Guru Meditation Error": These are often related to a stack overflow or a pointer issue. Either way they're usually easy to guess based on what's been changed along with a google search of the type of error thrown. If not shoot Tim a text.
If nodes aren't connecting to GUI: Confirm nodes are connecting to wifi and the GUI using the CLI. If you can reset the node without an error being thrown in the CLI, they successfully made a TCP connection with the GUI.