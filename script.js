const typedKey = document.querySelector(".typedKey");

function clearKeys(e) {
  let key_triggers = document.querySelectorAll("input[name=trigger]:checked");
  let mod_triggers = document.querySelectorAll("input[name=modtrigger]:checked");
  for (item of key_triggers) {
    item.checked = false;
  }
  for (item of mod_triggers) {
    item.checked = false;
  }
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
  let key = "None"
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
    if (count > 6)
      break;
    send_data[count] = checked_data.value;
    count++;
  }
  /*
  console.log("key1 = " + send_data[0]);
  console.log("key2 = " + send_data[1]);
  console.log("key3 = " + send_data[2]);
  console.log("key4 = " + send_data[3]);
  console.log("key5 = " + send_data[4]);
  console.log("key6 = " + send_data[5]);
  console.log("mod = " + send_data[6]);
  */

  create_senddata();
  document.getElementById("pending").textContent += text;
  cleanup();
}

let Layer_num = 0;
let key_num = 0;
function setLayerNum(e) {
  Layer_num = e.target.value - 1;
}

function setKeyNum(e) {
  key_num = e.target.value - 1;
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

async function readUntilClosed() {
  while (port.readable && keepReading) {
    reader = port.readable.getReader();
    try {
      while (true) {
        const { value, done } = await reader.read();
        if (done) {
          // reader.cancel() has been called.
          break;
        }
        // value is a Uint8Array.
        //console.log(value);
      }
    } catch (error) {
      // Handle error...
    } finally {
      // Allow the serial port to be closed later.
      reader.releaseLock();
    }
    await port.close();
  }
}
let text = "";
document.getElementById("send").addEventListener('click', async () => {

  create_senddata();
  var encoder = new TextEncoder();
  var ab8 = encoder.encode(text);
  console.log(text);

  const writer = port.writable.getWriter();
  const data = ab8;
  console.log(ab8);
  await writer.write(data);
  writer.releaseLock();
});

function toHex(v) {
  return '0x' + (('00' + v.toString(16).toUpperCase()).substring(v.toString(16).length));
}

function create_senddata() {
  text = "M_";
  text += Layer_num;
  text += "_";
  text += key_num;
  text += "_[";
  text += toHex(send_data[0]); text += ",";
  text += toHex(send_data[1]); text += ",";
  text += toHex(send_data[2]); text += ",";
  text += toHex(send_data[3]); text += ",";
  text += toHex(send_data[4]); text += ",";
  text += toHex(send_data[5]); text += ",";
  text += toHex(send_data[6]);
  text += "]\r\n";
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