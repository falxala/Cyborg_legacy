const typedKey = document.querySelector(".typedKey");

function clearKeys(e) {
  let key_triggers = document.querySelectorAll("input[name=trigger]:checked");
  let mod_triggers = document.querySelectorAll("input[name=modtrigger]:checked");
  let con_triggers = document.querySelectorAll("input[name=contrigger]:checked");
  let count = 0;
  for (item of key_triggers) {
    if (item.checked == true)
      count++;
    item.checked = false;
  }
  for (item of mod_triggers) {
    if (item.checked == true)
      count++;
    item.checked = false;
  }

  for (item of con_triggers) {
    if (item.checked == true)
      count++;
    item.checked = false;
  }

  if (e == true && count != 0)
    delete_last_line();
}

const send_data = new Uint8Array([0, 0, 0, 0, 0, 0, 0]);
function typed(e) {
  send_data[0] = 0;
  send_data[1] = 0;
  send_data[2] = 0;
  send_data[3] = 0;
  send_data[4] = 0;
  send_data[5] = 0;
  send_data[6] = 0;
  let key_triggers = document.querySelectorAll("input[name=trigger]:checked");
  let mod_triggers = document.querySelectorAll("input[name=modtrigger]:checked");
  let con_triggers = document.querySelectorAll("input[name=contrigger]:checked");
  let mod = "None"
  let mod_value = 0;
  for (let checked_data of mod_triggers) {
    if (mod == "None")
      mod = "";
    mod += checked_data.value;
    const view = new DataView(new ArrayBuffer(1));
    let bit_shift = checked_data.value - 224;
    mod_bit = 1;
    if (bit_shift > 0) {
      mod_bit = mod_bit << bit_shift;
      mod_value |= mod_bit;
    }
    else {
      mod_bit = 1;
      mod_value |= mod_bit;
    }
  }
  send_data[6] = mod_value;

  let count = 0;
  for (let checked_data of key_triggers) {
    if (count > 5)
      break;
    send_data[count] = checked_data.value;
    count++;
  }

  count = 0;
  for (let checked_data of con_triggers) {
    if (count > 0)
      break;
    send_data[0] = 255;
    send_data[1] = checked_data.value;
    count++;
  }

  create_senddata();
  document.getElementById("pending").textContent += textdata;
  cleanup();
}

let Layer_num = 0;
let key_num = 0;
function setLayerNum(e) {
  Layer_num = e.target.value;
  clearKeys();
}

function setKeyNum(e) {
  key_num = e.target.value - 1;
  clearKeys();
}


function openSerial(e) {
  SerialBegin();
}

var port;
var closedPromise;
async function SerialBegin() {
  // Prompt user to select any serial port.
  const filters = [
    { usbVendorId: 0xcafe },
    { usbVendorId: 0x239a },
  ];
  // Prompt user to select an Arduino Uno device.
  port = await navigator.serial.requestPort({ filters });

  await port.open({ baudRate: 115200 });

  keepReading = true;
  closedPromise = readUntilClosed();
};

let keepReading = true;
let reader;
let decoder = new TextDecoder()
let buffer = "";

async function readUntilClosed() {
  while (port.readable && keepReading) {
    reader = port.readable.getReader();
    try {
      const input = document.getElementById('send');
      input.disabled = false;
      const input2 = document.getElementById('connect');
      input2.disabled = true;

      while (true) {
        const { value, done } = await reader.read();
        if (done) {
          // reader.cancel() has been called.
          break;
        }
        // value is a Uint8Array.
        buffer += decoder.decode(value);
        let i = 0;
        for (let c of buffer) {
          if (c == '\n') {
            let line = buffer.slice(0, i).replace('\n', "");
            if (line != "") {
              readfunction(line);
            }
            buffer = buffer.slice(i);
            i = 0;
          }
          i++;
        }
      }
    } catch (error) {
      console.log(error);
      // Handle error...
    } finally {
      // Allow the serial port to be closed later.
      reader.releaseLock();
    }
    await port.close();
  }
}

function readfunction(messeage) {

  switch (parseInt(messeage.replace('lyr:', ''))) {
    case 0:
      document.getElementById("layer0").checked = true;
      Layer_num = 0;
      break;
    case 1:
      document.getElementById("layer1").checked = true;
      Layer_num = 1;
      break;
    case 2:
      document.getElementById("layer2").checked = true;
      Layer_num = 2;
      break;
    case 3:
      document.getElementById("layer3").checked = true;
      Layer_num = 3;
      break;
    case 4:
      document.getElementById("layer4").checked = true;
      Layer_num = 4;
      break;
    case 5:
      document.getElementById("layer5").checked = true;
      Layer_num = 5;
      break;
  }

  switch (parseInt(messeage.replace('kys:', ''))) {
    case 1:
      document.getElementById("key1").checked = true;
      key_num = 0;
      break;
    case 2:
      document.getElementById("key2").checked = true;
      key_num = 1;
      break;
    case 4:
      document.getElementById("key3").checked = true;
      key_num = 2;
      break;
    case 8:
      document.getElementById("key4").checked = true;
      key_num = 3;
      break;
    case 16:
      document.getElementById("key5").checked = true;
      key_num = 4;
      break;
    case 32:
      document.getElementById("key6").checked = true;
      key_num = 5;
      break;
  }

  if (messeage.toString().indexOf("enc:+") !== -1) {
    document.getElementById("keyR").checked = true;
    key_num = 6;
  }

  if (messeage.toString().indexOf("enc:-") !== -1) {
    document.getElementById("keyL").checked = true;
    key_num = 7;
  }


  clearKeys();
  console.log(messeage);
}

const wait = async (ms) => new Promise(resolve => setTimeout(resolve, ms));

document.getElementById("send").addEventListener('click', async () => {
  var pending = document.getElementById('pending').textContent.replace(/\r\n|\r/g, "\n");
  var queue = pending.split('\n');
  var encoder = new TextEncoder();
  for (var i = 0; i < queue.length; i++) {
    if (queue[i] == '') {
      continue;
    }
    const writer = port.writable.getWriter();
    var ab8 = encoder.encode(queue[i] + "\r\n");
    const data = ab8;
    writer.write(data);
    writer.releaseLock();
    await wait(50);
  }
  document.getElementById('pending').textContent = "";
});

function toHex(v) {
  return '0x' + (('00' + v.toString(16).toUpperCase()).substring(v.toString(16).length));
}

let textdata = "";
function create_senddata() {
  textdata = "M_";
  textdata += Layer_num;
  textdata += "_";
  textdata += key_num;
  textdata += "_[";
  textdata += toHex(send_data[0]); textdata += ",";
  textdata += toHex(send_data[1]); textdata += ",";
  textdata += toHex(send_data[2]); textdata += ",";
  textdata += toHex(send_data[3]); textdata += ",";
  textdata += toHex(send_data[4]); textdata += ",";
  textdata += toHex(send_data[5]); textdata += ",";
  textdata += toHex(send_data[6]);
  textdata += "]\r\n";
}

function cleanup() {
  var text = document.getElementById('pending').value.replace(/\r\n|\r/g, "\n");
  var lines = text.split('\n');
  var outArray = new Array();

  for (var i = 0; i < lines.length; i++) {
    // 空行は無視する
    if (lines[i] == '') {
      continue;
    }

    outArray.push(lines[i]);
  }
  var remove = new Array();
  for (var i = outArray.length - 1; i >= 0; i--) {
    for (var j = i - 1; j >= 0; j--) {
      if (outArray[i].slice(0, 6) == outArray[j].slice(0, 6))
        remove.push(j)
    }
  }
  new Set(remove).forEach(item => {
    outArray.splice(item, 1);
  });

  var newtext = "";
  outArray.forEach(line => {
    newtext += line;
    newtext += "\n";
  })
  document.getElementById('pending').textContent = newtext;
}

function delete_last_line() {
  var text = document.getElementById('pending').value.replace(/\r\n|\r/g, "\n");
  var lines = text.split('\n');
  var outArray = new Array();

  for (var i = 0; i < (lines.length - 2); i++) {
    if (lines[i] == '') {
      continue;
    }
    outArray.push(lines[i]);
  }

  var newtext = "";
  outArray.forEach(line => {
    newtext += line;
    newtext += "\n";
  })
  document.getElementById('pending').textContent = newtext;
}

