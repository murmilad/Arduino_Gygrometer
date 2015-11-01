#!/usr/bin/python2

#__author__ = 'Igor Maculan <n3wtron@gmail.com>'
import logging
import re, io, os, sys, codecs, json


from lockfile.pidlockfile import PIDLockFile
import daemon, lockfile, serial, time

from datetime import datetime, date

import matplotlib

matplotlib.use('Agg')
#matplotlib.rcParams['timezone'] = 'Europe/Moscow'  # Replace with your favorite time

from mpl_toolkits.axes_grid1 import host_subplot
import mpl_toolkits.axisartist as AA

import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter
from matplotlib import style

logging.basicConfig(level=logging.DEBUG)

graph_period_days = 1

#initialization and open the port

#possible timeout values:

#     1. None: wait forever, block call

#     2. 0: non-blocking mode, return immediately

#     3. x, x is bigger than 0, float allowed, timeout block call
def as_config(dct):
    return Dummy(
                info_gygrometer_file=dct['info_gygrometer_file'],
                info_temperature_file=dct['info_temperature_file'],
                info_co2_file=dct['info_co2_file'],
                spool_folder=dct['spool_folder'],
                image_folder=dct['image_folder']
        )

class Dummy(object):
    def __init__(self, **kwargs):
        self.__dict__.update(kwargs)

def main():
    style.use('dark_background')

    config_file = sys.argv[1]
    with open(config_file) as conf:
        config = json.load(conf, object_hook=as_config)

    ser = serial.Serial()

    with open('/var/arduino_device', 'r') as device:
        ser.port = device.read().rstrip('\n')
    
    ser.baudrate = 9600
    
    ser.bytesize = serial.EIGHTBITS #number of bits per bytes
    
    ser.parity = serial.PARITY_NONE #set parity check: no parity
    
    ser.stopbits = serial.STOPBITS_ONE #number of stop bits
    
    #ser.timeout = None             #block read
    
    ser.timeout = 0                 #non-block read
    
    #ser.timeout = 2                  #timeout block read
    
    ser.xonxoff = False      #disable software flow control
    
    ser.rtscts = False      #disable hardware (RTS/CTS) flow control
    
    ser.dsrdtr = False         #disable hardware (DSR/DTR) flow control
    
    ser.writeTimeout = 2      #timeout for write
    
    print "serial port: "

    try: 
    
        ser.open()
    
    except Exception, e:
    
        print "error open serial port: " + str(e)
    
        exit()
    
    if ser.isOpen():

        try:

              ser.flushInput() #flush input buffer, discarding all its contents

              ser.flushOutput()#flush output buffer, aborting current output 

              #and discard all that is in buffer

              #write data

              #ser.write("AT+CSQ=?\x0D")

              #print("write data: AT+CSQ=?\x0D")

              #time.sleep(0.5)  #give the serial port sometime to receive the data

              numOfLines = 0

              # Read the data file
              data = []
              dates = []
              tempdata = []
              hydata = []
              co2data = []



              is_file_read = False
              sleep_time = 2
              line = ""

              while True:
                    response = ser.readline()
                    if is_file_read == False:
                        time.sleep(2)
                        match = re.search('^([\d\.]+),([-\d\.]+),([\d\.]+),([<\d]+)', response)
                        if match:
                            print("values: " + match.group(1) + " " + match.group(2) + " " + match.group(3))
                            with open(config.info_gygrometer_file, 'w') as info:
                                info.write(match.group(2))
                            with open(config.info_temperature_file, 'w') as info:
                                info.write(match.group(1))
                            with open(config.info_co2_file, 'w') as info:
                                info.write(match.group(4))
                        else:
                            match = re.search('log begin (\d+)', response)
                            if match:
                                timestamp = str(time.time())
                                graph_period_days = int(match.group(1)) - 1;
                                with open(config.info_gygrometer_file, 'w') as info:
                                    info.write('Reading...')
                                with open(config.info_temperature_file, 'w') as info:
                                    info.write('Reading...')
                                with open(config.info_co2_file, 'w') as info:
                                    info.write('Reading...')
                                with codecs.open(config.spool_folder + '/' + timestamp, 'w', encoding='utf8') as msg:
                                    json.dump({
                                        'type': 'arduino',
                                        'title': 'Arduino',
                                        'body': 'Start statistics reading',
                                        'img': config.image_folder + '/arduino.png'
                                    }, msg, ensure_ascii=False)
                                    os.chmod(config.spool_folder + '/' + timestamp, 0666)

                                print("+" + response)
                                is_file_read = True
                                sleep_time = 0                            
                    else:
                        match = re.search('\n', response)
                        line = line + response
                        time.sleep(0.02);
                        if match:
                            match = re.search('(\d+:\d+:\d+ \d+\.\d+\.\d+),([-\d\.]+),([\d\.]+),([\d\.]+),([<\d]+)', line)
                            if match:
                                dates.append(matplotlib.dates.date2num(datetime.strptime(match.group(1),"%H:%M:%S %d.%m.%Y")))
                                tempdata.append(match.group(2))
                                hydata.append(match.group(3))
                                co2data.append(match.group(5))
#                                with open(config.info_gygrometer_file, 'w') as info:
#                                    info.write('Reading...')
#                                with open(config.info_temperature_file, 'w') as info:
#                                    info.write('Reading...')
#                                with open(config.info_co2_file, 'w') as info:
#                                    info.write('Reading...')
#                                line = 'log end'

                            match = re.search('log end', line)
                            if match:
                                print("+"+ line)
                                is_file_read = False


                                # Set up the plot
                                host = host_subplot(111, axes_class=AA.Axes)

                                #plt.figure(figsize=(10,6))
                                plt.subplots_adjust(right=0.75)

                                par1 = host.twinx()


                                par2 = host.twinx()
                                new_fixed_axis = par2.get_grid_helper().new_fixed_axis
                                par2.axis["right"] = new_fixed_axis(loc="right",
                                                        axes=par2,
                                                        offset=(60, 0))

                                par2.axis["right"].toggle(all=True)



                                host.set_xlabel("Time")
                                host.set_ylabel("Temperature C")
                                par1.set_ylabel("Humidity %")
                                par2.set_ylabel("CO2 ppm")

                                p1, = host.plot_date(dates, tempdata, ls="-")
                                p2, = par1.plot_date(dates, hydata, ls="-" )
                                p3, = par2.plot_date(dates, co2data, ls="-")

                                host.set_xlim(dates[-1]-graph_period_days, dates[-1])

                                host.xaxis.set_major_formatter( DateFormatter( '%H:%M' ) )
#                                par1.xaxis.set_major_formatter( DateFormatter( '%H:%M' ) )
#                                par2.xaxis.set_major_formatter( DateFormatter( '%H:%M' ) )

                                host.legend()

                                host.axis["left"].label.set_color(p1.get_color())
                                par1.axis["right"].label.set_color(p2.get_color())
                                par2.axis["right"].label.set_color(p3.get_color())

                                        
#                                plt.tight_layout()
                              
                                # Save the image to buffer
                                os.remove(config.image_folder + '/arduino_graph.png')
                                plt.savefig(config.image_folder + '/arduino_graph.png', format='png')
                                with codecs.open(config.spool_folder + '/' + timestamp, 'w', encoding='utf8') as msg:
                                    json.dump({
                                        'type': 'arduino',
                                        'title': 'Arduino',
                                        'body': 'Statistics graph',
                                        'img': config.image_folder + '/arduino_graph.png'
                                    }, msg, ensure_ascii=False)
                                    os.chmod(config.spool_folder + '/' + timestamp, 0666)
                                print 'Created graph\n'

                                plt.clf()
                                plt.cla()
                                plt.close('all')
                                data = []
                                dates = []
                                tempdata = []
                                hydata = []
                                co2data = []
                            line = "";

                    #if (numOfLines >= 5):

                         #break

                         #ser.close()

        except Exception, e1:

              print "error communicating...: " + str(e1)

    else:
    
        print "cannot open serial port "
    

if __name__ == '__main__':
     with daemon.DaemonContext(
          pidfile=PIDLockFile('/var/run/arduino_sensors.pid'),
          stdout=sys.stdout
    ):
         main()
#main()
