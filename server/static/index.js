"use strict";

let fileInput = document.querySelector("#file_input"),
    progressSpan = document.querySelector("#progress_status_span"),
    spinnerDiv = document.querySelector(".loading_spinner"),
    progressDiv = document.querySelector(".progress_bar"),
    fileSelectDiv = document.querySelector(".file_selection"),
    resultsDiv = document.querySelector(".results"),
    splashSpan = document.querySelector("#splash_text");

fileInput.addEventListener("change", function() {
    let file = fileInput.files[0];
    upload(file);
});

document.querySelector("#upload_submit").addEventListener("click", function() {
    fileInput.click();
});

function updateSplash(text) {
    let emojicons = ["🔥", "👊", "💤", "🐻", "👊", "🍔", "🏆", "😎", "📖", "🆒"];
    let idx = Math.floor(Math.random() * emojicons.length);
    splashSpan.innerHTML = "<h2>" + text + " " + emojicons[idx] + "</h2>";
}

function upload(file) {
    let form = new FormData();
    let xhr = new XMLHttpRequest();
    updateSplash("Uploading " + file.name);
    fileSelectDiv.style.display = 'none';
    progressDiv.style.display = 'block';

    function updateProgress(percentage) {
        progressSpan.setAttribute("style", "width: " + percentage + "%");
        if (percentage >= 100) {
            // Show next stage, the spinner.
            updateSplash("Processing " + file.name);
            progressDiv.style.display = 'none';
            spinnerDiv.style.display = 'block';
        }
    }

    function groupBy(array, interval) {
        let groups = [];
        let group = [];
        for (let i = 0; i < array.length; i++) {
            group.push(array[i]);
            if ((i + 1) % interval == 0) {
                groups.push(group);
                group = [];
            }
        }
        if (group.length > 0) {
            groups.push(group);
        }
        return groups;
    }

    xhr.upload.addEventListener("progress", function(e) {
        if (e.lengthComputable) {
            // Update progress bar.
            var percentage = Math.round((e.loaded * 100) / e.total);
            updateProgress(percentage);
        }
    }, false);

    xhr.upload.addEventListener("load", function(e){
        // Upload complete.
        updateProgress(100);
    }, false);

    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            // File uploaded and response received, display results and hide spinner.
            let response = JSON.parse(xhr.responseText);
            // Put linked titles first.
            response.titles.sort(function(a, b) {
                return b.results.length - a.results.length;
            });
            let columns = [];
            for (let i = 0; i < response.titles.length; i++) {
                let result = response.titles[i];
                let col = document.createElement('td');
                col.innerHTML = `<img title="${result.text}" src=${result.img}>`;
                if (result.results.length === 0) {
                    col.innerHTML += `<p>${result.text}`;
                } else {
                    for (let i = 0; i < result.results.length; i++) {
                        let res = result.results[i];
                        col.innerHTML += `<p><a href=${res.link}>${res.title}</a>`;
                    }
                }
                columns.push(col);
            }
            let groupedCols = groupBy(columns, 2);
            console.log(groupedCols);
            let resultsTable = document.querySelector("#results_table");
            for (let r = 0; r < groupedCols.length; r++) {
                let row = document.createElement('tr');
                for (let rr = 0; rr < groupedCols[r].length; rr++) {
                    row.appendChild(groupedCols[r][rr]);
                }
                resultsTable.appendChild(row);
            }
            updateSplash("Here you go!");
            spinnerDiv.style.display = 'none';
            resultsDiv.style.display = 'block';
        }
    };

    xhr.open("POST", "/post/", true);
    form.append("image", file);
    xhr.send(form);
}

updateSplash("Hello, show me your books.");
