import mtca4u
from array import *

mtca4u.set_dmap_location("../bam_motor.dmap")

motorDevice = mtca4u.Device("MOTOR_DEVICE")
 
motorDevice.write("BOARD0", "WORD_RESET_N", 1);

