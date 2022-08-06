/*
 *
 * NAME:        Guanhong Li
 *
 * HOMEWORK:    1
 *
 * CLASS:       ICS 451
 *
 * INSTRUCTOR:  Ravi Narayan
 *
 * DATE:        September 8, 2021
 *
 * FILE:        MyClient.java
 *
 * DESCRIPTION: This file contains the client for homework 1.
 *
 */
import java.net.Socket;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.PrintStream;

public class MyClient {
  
    /**
     * Starts a simple client which receives a text file from a simple server.
     * @param args the command line arguments
     * @throws IOException
     */
    public static void main(String[] args) throws IOException {
        String IPAddress = "127.0.0.1";
        int portNumber = Integer.parseInt(args[0]);
        String fileLine;
        
        PrintStream clientLog = new PrintStream("client_log.txt");
        
        clientLog.println("Starting connection...");
        Socket clientSocket = new Socket(IPAddress, portNumber);
        clientLog.println("Connected to server at port " + portNumber + ".");
        
        clientLog.println("Receiving message from server...");
        BufferedReader clientInput = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));
        PrintStream fileOutput = new PrintStream("client_file.txt");
        while ((fileLine = clientInput.readLine()) != null) {
            fileOutput.println(fileLine);
        }
        clientLog.println("Message received from server.");
        
        fileOutput.close();
        clientInput.close();
        
        clientLog.println("Closing connection...");
        clientSocket.close();
    }
}