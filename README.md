# iotFirstResponder
Senior Design Project to create an IoT system that can be used to fight fires

## Quick Start
1. Connect your computer to a Wi-Fi network
2. Run the GUI and check what the IP Address listed is(That is your computer's IP)
3. In wifi_task.cpp, edit the wifi_task::host_ip variable to match your computer's IP
4. In wifi_task.cpp, edit function wifi_task::wifi_task() such that _ssid and _pwd match your current Wi-Fi credentials
5. Upload the firmware to the ECMUs
6. On the GUI initialization window, set it to one node and press start
7. Open the serial terminal on the node and type in "demo" and press enter
8. Record the identifier for the node listed by the python terminal(You should write it on the device or on a piece of tape as well)
9. Repeat recording the identifier for each node (steps 6-8)
10. Add a row to nodeButtons.csv for each node. (x,y give the location in the GUI)
11. For each output attached to each node, fill in the information in nodeOutputAssignments.csv and doorButtons.csv. The former is for the network and the latter is to create the GUI objects
12. Rerun the GUI but enter the total number of nodes being used and press start
13. Connect a serial monitor to each node
14. While each node is attached to serial, type "demo" and press enter
15. Enjoy!
