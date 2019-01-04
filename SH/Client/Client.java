

import java.io.BufferedReader;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.EOFException;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStreamReader;
import java.net.Socket;
import java.util.Scanner;

class Client{  
public static void main(String args[])throws Exception{  
String address = "localhost";
/*Scanner sc=new Scanner(System.in);
System.out.println("Enter Server Address: ");
address=sc.nextLine();*/
//create the socket on port 5000
Socket s=new Socket(address,5000);  
DataInputStream din=new DataInputStream(s.getInputStream());  
DataOutputStream dout=new DataOutputStream(s.getOutputStream());  
BufferedReader br=new BufferedReader(new InputStreamReader(System.in));  
Scanner sc=new Scanner(System.in);
String filename;  
//try{
while (sc.hasNextLine()){

  System.out.println("Enter File Name: ");
filename=sc.nextLine();
 
dout.writeUTF(filename);  
dout.flush(); 
	System.out.println("Receving file: "+filename);
	
	System.out.println("Saving as file: "+filename);

long sz=Long.parseLong(din.readUTF());
System.out.println ("File Size: "+sz+" kb");

byte b[]=new byte [1024];
System.out.println("Receving file..");
FileOutputStream fos=new FileOutputStream(new File(filename),false);
int bytesRead=0;
long test=0;
sz = sz+1024;
while((sz-1024)>=0){
 int a = (int)sz % 1024;
 if((sz-1024)>1024){
bytesRead = din.read(b, 0,1024);
}
else{
  bytesRead = din.read(b, 0,a);
}

fos.write(b,0,bytesRead);
test++;
sz = sz - 1024;
}
System.out.println("bytesRead"+test+"last" + bytesRead);
System.out.println("Comleted");

fos.close();
}
dout.close();  	
s.close();  
/*}
catch(EOFException e)
{
	//do nothing
}*/
}
}  
//this is a typical client program 
