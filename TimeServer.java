       

import java.net.*;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Calendar;
import java.util.Date.*;

import java.awt.Event.*;

public class TimeServer 
{

//HOUR:MIN:SEC:MONTH:DAY:YEAR
    
    public static void main(String [] args) throws IOException
    {
        int port = 123;
                
        ServerSocket serverSocket;
        Socket socket;
        InputStream in = null;
        OutputStream out = null;
        
	
        
        //Arguments set the port
        if (args != null)
        {
            if (args.length > 0)
            {
                port = Integer.parseInt(args[0]);
            }
        }
            
        System.out.println("Server starting...");
        
        try
        {
            InetAddress addy = InetAddress.getLocalHost();
            System.out.println("I'm at: " + addy.getHostAddress() + ":" + port + "\n");
        }
        catch (UnknownHostException e)
        {
            System.out.println("SERVER: Problem with resolving local host...");
        }
        
        serverSocket = new ServerSocket(port);
        
        System.out.println("Ready...");
        
        //Listen for socket connection requests; create a new socket and pass
        //it to a new ClientThread 
        while (true)
        {
            socket = serverSocket.accept();
            
            
            if (socket.isConnected() == true)
            {
                System.out.println("Connection...");
                
                try
                {
                    
                    in = socket.getInputStream();
                    out = socket.getOutputStream();

                }
                catch (IOException e)
                {
                    System.out.println("ClientThread: error opening streams and sockets");
                }
                

                
                String timeString = Calendar.getInstance().getTime().toString();
                System.out.println("RAW STRING: " + timeString);
                
                //Parse the time string into ints for data transport
                String dataString = "";
                
                /*
                    HOUR:MINUTE:SECOND:DAY:MONTH:YEAR
                */
                
                //HOUR
                String Hour = timeString.substring(11,13);
                dataString += Hour;
                //MINUTE
                String Minute = timeString.substring(14,16);
                dataString += ":" + Minute;
                //SECONDS
                String Second = timeString.substring(17,19);
                dataString += ":" + Second;

                //MONTH
                String  Month = timeString.substring(4,7);
                //Interp Month as int
                String intMonth = "";
                
                switch (Month)
                {
                    case "Jan":
                        intMonth = "01";
                        break;
                    case "Feb":
                        intMonth = "02";
                        break;
                    case "Mar":
                        intMonth = "03";
                        break;
                    case "Apr":
                        intMonth = "04";
                        break;
                    case "May":
                        intMonth = "05";
                        break;
                    case "Jun":
                        intMonth = "06";
                        break;
                    case "Jul":
                        intMonth = "07";
                        break;
                    case "Aug":
                        intMonth = "08";
                        break;
                    case "Sep":
                        intMonth = "09";
                        break;
                    case "Oct":
                        intMonth = "10";
                        break;
                    case "Nov":
                        intMonth = "11";
                        break;
                    case "Dec":
                        intMonth = "12";
                        break;
                    default:
                        intMonth = "99";
                        break;                        
                } //swicth

                dataString += ":" + intMonth;
                
                //DAY
                String Day = timeString.substring(8,10);
                dataString += ":" + Day;

                //YEAR
                String Year = timeString.substring(26,28);
                dataString += ":" + Year;
                
                System.out.println("output: " + dataString);
                
                out.write(dataString.getBytes());
                
                socket.close();
                in.close();
                out.close();
                socket = null;
		
                System.out.println("Served!");
            }
        }
        
    }
    
}  


