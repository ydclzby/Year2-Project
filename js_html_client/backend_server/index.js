const express = require("express"); 
const cors = require('cors');
const path = require('path');
const PORT = process.env.PORT || 3001; 
const app = express();
var mysql = require('mysql');

var con = mysql.createConnection(
    {
        host:"3.86.16.180",
        user:"bz1521",
        passward:"Zby020927.",
        database:"Persondb"
    });

con.connect(function(err){
    if(err) throw err;
    console.log("Successfully connected to the database..\n");
});

app.get("/personQuery", (req,res)=>{
    con.query("SELECT *FROM Person", function (err,result,fields){
        if(err) throw err;
        res.json(result)
    });
});

app.use(cors({ origin: '*'
}));

app.use(express.static(path.resolve(__dirname, './client/build')));


app.use(cors({
    methods: ['GET','POST','DELETE','UPDATE','PUT','PATCH']
    }));

app.get("/tableData33", (req, res) => {
    setTimeout(() =>{ res.json(
    {
        "tableData33":[
            ['Ed', 15 + Math.floor(Math.random() * 35), 'Male'], ['Mia', 15 + Math.floor(Math.random() * 35), 'Female'], ['Max', 15 + Math.floor(Math.random() * 35), 'Male']
        ]
    })}, 8000);
});

app.get('*', (req, res) => {
    res.sendFile(path.resolve(__dirname, './client/build', 'index.html'));
});

app.listen(PORT, () => {
    console.log(`Server listening on ${PORT}`);
});