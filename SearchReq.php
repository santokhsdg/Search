<?php
function Search($req)
{
$req=trim($req);
$reqid=uniqid('req'); 
$result=exec("./smm $reqid $req");    
     
//$result=1;
     if($result==1)
         {  // GIVE PATH FROM THE ROOT
           $path="/CORE/SearchOutput/".$reqid.".txt";
           $fl = fopen($path,"r") or die("Unable to open file!");
           $fr = fread($fl,filesize($path));
           trim($fr);
           fclose($fl);        
           $remove = "\n";
           $res = explode($remove, $fr);
           unlink($path); 
            if($res==""){
                return "No Result found";
            }
             else{
                 return $res;
             }
           
         }
     else
         {
          return "No Result found";
         }    
    
}

?>