<?php


function UpdateTrieVal($string)
{  /// add entry to trie table on updation
    include("DBConnection.php");
    $remove = " ";
    $res = explode($string, $remove);
    $num=count($res);
    for($i=0;$i<num;$i++)
    {        
        $str=$res[$i];
        if($str!="")
        { //NAME`, `LOCATION`, `ARTIST1
        $sql="UPDATE trie set PREF=PREF+0.00001 where WORD='$str'";
        $ans=mysqli_query($con,$sql);                 
        }
        
    }
    
}


function UpdateTriePref($ID)
{  /// add entry to trie table on updation
    include("DBConnection.php");    
    $sql="SELECT * from song where SONG_ID='$ID'";
    $row=mysqli_fetch_assoc($sql);
    UpdateTrieVal($row["NAME"]);
    UpdateTrieVal($row["ALBUM_NAME"]);
    UpdateTrieVal($row["TYPE"]);
    UpdateTrieVal($row["ARTIST1"]); UpdateTrieVal($row["ARTIST2"]); UpdateTrieVal($row["ARTIST3"]);
    
    $sql="SELECT * from song_info where SONG_ID='$ID'";
    $row=mysqli_fetch_assoc($sql);
    UpdateTrieVal($row["BAND"]);
    
}
?>