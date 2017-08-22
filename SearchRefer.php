<?php

if($_POST)
{
$req =$_POST['request'];
$req=trim($req);
$reqid=uniqid('req');
$output="";
include('DBConnection.php');
$result=exec("./smm $reqid $req");
//$result=0;
     if($result==1)
         {  // PATH IS REQUIRED FROM THE ROOT
           $path="/CORE/SearchOutput/".$reqid.".txt";
           $file = fopen($path,"r") or die("Unable to open file!");      
           $fr = fread($file,filesize($path));
           fclose($file);
           $remove = "\n";
            trim($fr); 
            $res = explode($remove, $fr);
            unlink($path);
           $cnt=count($res);
           $to_encode = array();
           for($i=0;$i<$cnt-1;$i++)
           { 
           $sql = "SELECT SONG_ID,NAME,ARTIST1,ARTIST2,ARTIST3,ALBUM_NAME,GENRE FROM song WHERE SONG_ID='$res[$i]'";
           $output=mysqli_query($con,$sql);          
           $row = mysqli_fetch_assoc($output);
           $row["SONG_ID"]=base64_encode($row["SONG_ID"]);
               //echo $row['NAME'];
           array_push($to_encode,$row);
           }
           mysqli_close($con);
           $data=array("result" => $to_encode);            
           echo json_encode($data);           
         }
     else
         {
          echo ""; //error
         }    
    

}

?>
