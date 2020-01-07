import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Universal 2.12
import QtQuick.Layouts 1.13

import com.ac.socket 1.0

ApplicationWindow {
    visible: true

    minimumHeight: 480
    minimumWidth: 640

    property bool is_log_in: false

    Component.onCompleted: {
        var rec_size = db.getMaxID();
        var text, due, done;
        for (var i = 1; i <= rec_size; i++)
        {
            text = db.getText(i);
            done = db.getIsDone(i);
            list_model.append_rec(done, i, text);
        }
    }

    width: 640
    height: 480
    title: "Memo"

    background: Rectangle {
        id: bg
        color: "#262626"
    }

    ListModel {
        id: list_model
        function delete_all_fin()
        {
            console.log(count);

            var i = 0;
            var is_removed;
            while (i < count)
            {
                console.log(get(i).is_fin);
                if (get(i).is_fin)
                {
                    is_removed = db.removeRecord(get(i).rec_id);
                    if (is_removed)
                    {
                        remove(i);
                        update_list();
                    }
                    else
                    {
                        console.log("Failed to remove record" + get(i));
                        msg_box.showMsgBox("Failed", "Cannot remove the records.")
                        break;
                    }
                }
                else
                    i++;
            }
        }

        function update_list()
        {
            for (var i = 0; i < count; i++)
                get(i).rec_id = i + 1;
        }

        function recsJsonStr()
        {
            var obj = {"EventGroup": "Data", "Event": "SyncFromClient", "ID": db.getId(), "Records": []};
            var rec;
            for (var i = 0; i < count; i++)
            {
                var rec_obj = {"RecID": 0, "Text": "", "DueDate": "1970-01-01", "Done": false};
                rec = get(i);
                rec_obj.RecID = rec.rec_id;
                rec_obj.Text = rec.rec_text;
                rec_obj.Done = rec.is_fin;
                obj.Records.push(rec_obj);
            }
            return JSON.stringify(obj);
        }

        function delete_all()
        {
            while(count)
            {
                db.removeRecord(count);
                remove(count-1);
            }
        }

        function append_rec(is_fin, rec_id, rec_text)
        {
            append({is_fin: is_fin, rec_id: rec_id, rec_text: rec_text});
        }

    }

    Text {
        anchors.centerIn: scroll_view
        text: "Click add button to create a memo"
        font.family: "Microsoft YaHei"
        font.pointSize: 25
        font.bold: true
        color: "#a6a6a6"
        visible: list_model.count ? false : true
    }

    ScrollView {
        id: scroll_view
        anchors.fill: parent

        ScrollBar.vertical.policy: ScrollBar.AlwaysOff

        ListView {
            id: list_view
            width: parent.width + 25
            model: list_model

            delegate: ItemDelegate
            {
                id: delegate_item
                width: parent.width
                height: 50

                background: Rectangle {
                    color: "#FAFAFA"
                    border.color: "#262626"
                    border.width: 2
                }

                CheckBox {
                    id: check
                    checked: false
                    text: ""
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    onCheckedChanged: {
                        is_fin = checked;
                        if (checked == true)
                        {
                            db.setIsDone(rec_id, true);
                            edit.focus = false;
                        }
                        else
                        {
                            db.setIsDone(rec_id, false);
                            edit.focus = true;
                        }
                        console.log("check.checked: " + checked + ", is_fin: " + is_fin);
                    }

                    Component.onCompleted: checked = is_fin;
                }

                Label {
                    id: lbl
                    anchors.left: check.right
                    anchors.right: downsize.left
                    anchors.top: parent.top
                    anchors.topMargin: 5

                    text: edit.length ? (edit.length > 15 ? edit.text.substring(0, 15) + "......" : edit.text) : "Waiting for input......"
                    font.family: edit.font.family
                    font.pointSize: edit.font.pointSize
                    font.strikeout: edit.font.strikeout

                    visible: !edit.visible

                }

                ToolButton {
                    id: downsize
                    property bool now_state: false
                    antialiasing: true
                    anchors.right: parent.right
                    anchors.rightMargin: 5
                    anchors.top: parent.top
                    anchors.topMargin: 10
                    height: check.height
                    width: check.width
                    text: now_state ? ">" : "V"
                    font.bold: true
                    font.family: "FiraCode"

                    onClicked: {
                        now_state = !now_state;
                        if (now_state)
                        {
                            if (check.checked != true)
                            {
                                edit.focus = true;
                            }
                            edit.cursorPosition = edit.length;

                            delegate_item.state = "expanded";
                            left_words.visible = true;
                            edit.visible = true;
                        }
                        else
                        {
                            db.setText(rec_id, edit.text);
                            delegate_item.state = "";
                            edit.focus = false;
                            left_words.visible = false;
                            edit.visible = false;
                        }
                        // flick.debug();
                    }
                }

                Flickable {
                    id: flick

                    anchors.left: check.right
                    anchors.right: downsize.left
                    anchors.top: parent.top
                    anchors.topMargin: 5
                    anchors.bottom: parent.bottom

                    contentWidth: edit.paintedWidth
                    contentHeight: edit.paintedHeight
                    clip: true
                    interactive: false
                    boundsMovement: Flickable.StopAtBounds

                    onContentYChanged: {
                        if (downsize.checked == false)
                            contentY = 0;
                    }

                    function debug()
                    {
                        console.log(width + ", " + height + ", " + contentHeight + ", " + contentWidth);
                    }

                    function ensureVisible(r)
                    {
                        if (contentX >= r.x)
                            contentX = r.x;
                        else if (contentX+width <= r.x+r.width)
                            contentX = r.x+r.width-width;
                        if (contentY >= r.y)
                            contentY = r.y;
                        else if (contentY+height <= r.y+r.height)
                            contentY = r.y+r.height-height;
                    }

                    TextEdit {
                        id: edit

                        property int length_bounder: 200

                        visible: false;
                        width: flick.width
                        height: flick.height
                        selectByMouse: true
                        activeFocusOnPress: false
                        cursorVisible: false
                        textFormat: TextEdit.PlainText
                        focus: false
                        text: ""
                        font.family: "Microsoft YaHei"
                        font.pointSize: 20
                        font.strikeout: is_fin
                        wrapMode: TextEdit.Wrap

                        onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
                        onLengthChanged: {
                            var before_position = cursorPosition > length_bounder ? length_bounder : cursorPosition;
                            if (length > 120)
                            {
                                text = text.substring(0, length_bounder);
                                cursorPosition = before_position;
                            }
                            rec_text = text;
                        }

                        Component.onCompleted: text = rec_text;
                    }

                }

                states: [
                    State {
                        name: "expanded"
                        PropertyChanges { target: delegate_item; height: ((edit.contentHeight + 5) >= 200) ? edit.contentHeight + 5 : 200 }
                        PropertyChanges { target: flick; contentY: 0 }
                    }
                ]

                Text {
                    id: left_words
                    anchors.right: downsize.left
                    anchors.bottom: delegate_item.bottom
                    anchors.rightMargin: 5
                    anchors.bottomMargin: 5
                    text: String(edit.length_bounder - edit.length)
                    visible: false
                }


                transitions: [
                    Transition {
                        NumberAnimation {
                            duration: 200;
                            properties: "height, contentY"
                        }
                    }
                ]

            }
        }
    }

    menuBar:  ToolBar{
        position: ToolBar.Header
        RowLayout {
            ToolButton {
                id: add_btn
                text: "Add"
                height: parent.height
                onClicked: {
                    var is_added = db.addRecord("1970-01-01", "");
                    if (is_added)
                        list_model.append_rec(false, db.getMaxID(), "");
                    else
                        msg_box.showMsgBox("Failed", "Failed to add record.");
                }
            }
            ToolButton {
                id: del_sel_btn
                text: "DelSel"
                height: parent.height
                onClicked: list_model.delete_all_fin();
            }
            ToolButton {
                id: del_all_btn
                text: "DelAll"
                height: parent.height
                onClicked: list_model.delete_all();
            }
            ToolButton {
                id: syn_to_btn
                text: "SyncTo"
                height: parent.height

                signal received();

                onReceived: {
                    var obj = JSON.parse(socket.response);
                    if (obj.Event == "SyncReply" && obj.SyncResult)
                    {
                        msg_box.showMsgBox("Success", "You have sync your data to server.");
                    }
                    else
                    {
                        msg_box.showMsgBox("Failed", "Failed to sync your data to server.");
                    }
                }

                onClicked: {
                    if (is_log_in)
                    {
                        if (!socket.is_connected)
                        {
                            if (!socket.connect())
                            {
                                msg_box.showMsgBox("Failed", "Failed to connect to the server.");
                                return;
                            }
                        }

                        socket.setCallBack(syn_to_btn.received);
                        socket.write(list_model.recsJsonStr());
                    }
                    else
                    {
                        msg_box.showMsgBox("Failed", "Log in before sync your data to the server.")
                    }
                }
            }
            ToolButton {
                id: syn_from_btn
                text: "SyncFrom"
                height: parent.height

                property var sync_from_request_obj: {"EventGroup": "Data","Event": "SyncFromServer","ID": ""}

                signal received();

                onReceived: {
                    var obj = JSON.parse(socket.response);
                    if (obj.Event == "SyncData" && obj.ID == db.getId())
                    {
                        list_model.delete_all();
                        var recs = obj.Records;
                        for (var i = 0; i < recs.length; i++)
                        {
                            var is_added = db.addRecord("1970-01-01", recs[i].Text);
                            if (is_added)
                                list_model.append_rec(recs[i].Done, recs[i].RecID, recs[i].Text);
                            else
                            {
                                msg_box.showMsgBox("Failed", "Cannot insert some records.");
                                break;
                            }
                        }
                    }
                    else
                    {
                        msg_box.showMsgBox("Failed", "Failed to sync data from server.");
                    }

                }

                onClicked: {
                    if (is_log_in)
                    {
                        if (!socket.is_connected)
                        {
                            if (!socket.connect())
                            {
                                msg_box.showMsgBox("Failed", "Failed to connect to the server.");
                                return;
                            }
                        }

                        socket.setCallBack(syn_from_btn.received);
                        sync_from_request_obj.ID = db.getId();
                        socket.write(JSON.stringify(sync_from_request_obj));
                    }
                    else
                    {
                        msg_box.showMsgBox("Failed", "Log in before sync from the server.")
                    }

                }
            }
            ToolButton {
                id: log_in_btn
                text: "LogIn"
                height: parent.height
                onClicked: {
                    log_in_dialog.open()
                }

                Popup {
                    id: log_in_dialog

                    modal: true

                    closePolicy: Popup.NoAutoClose

                    width: 400
                    height: 300

                    parent: Overlay.overlay
                    anchors.centerIn: parent

                    contentItem: Rectangle {
                        id: content_item
                        height: parent.height
                        width: parent.width
                        color: "transparent"
                        anchors.centerIn: parent

                        signal received();

                        onReceived: {
                            console.log("Received response " + socket.response);
                            var obj = JSON.parse(socket.response);

                            if (obj.Result)
                            {
                                is_log_in = true;
                                var is_set = db.setID(account_input_textinput.text);
                                if (is_set)
                                {
                                    msg_box.showMsgBox("Success", "Log in successfully");
                                    account_input_textinput.text = ""
                                    password_input_textinput.text = ""
                                    log_in_dialog.close();
                                }
                                else
                                    msg_box.showMsgBox("Failed", "Failed to log in.");
                            }
                            else
                            {
                                msg_box.showMsgBox("Failed", "Incorrect password.");
                            }
                        }

                        Rectangle
                        {
                            anchors.top: parent.top
                            anchors.right: parent.right
                            anchors.bottom: inputs.top
                            anchors.left: parent.left
                            color: "transparent"
                            Text {
                                anchors.centerIn: parent
                                text: "Log In"
                                font.family: "Helvetica"
                                font.bold: true
                                font.pointSize: 25
                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.top: parent.top

                            }
                        }

                        Rectangle {
                            id: inputs
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: btns.top
                            height: 130
                            color: "transparent"


                            Rectangle {
                                id: account_input
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: parent.height / 2
                                color: "transparent"
                                Rectangle {
                                    id: account_input_label
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.bottom: parent.bottom
                                    width: parent.width / 4
                                    color: "transparent"
                                    Label {
                                        anchors.centerIn: parent
                                        text: "Account"
                                        font.family: "Helvetica"
                                        font.pointSize: 15

                                    }
                                }

                                Rectangle {
                                    anchors.fill: account_input_textinput
                                    color: "#ffffff"
                                }

                                TextInput {
                                    id: account_input_textinput
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: account_input_label.right
                                    anchors.leftMargin: 10
                                    anchors.right: parent.right
                                    font.family: "Helvetica"
                                    font.pointSize: 13
                                    validator: RegExpValidator { regExp: /[0-9a-zA-Z]{0,22}/ }
                                }
                            }

                            Rectangle {
                                id: password_input
                                anchors.top: account_input.bottom
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.bottom: parent.bottom
                                color: "transparent"
                                Rectangle {
                                    id: password_input_label
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.bottom: parent.bottom
                                    width: parent.width / 4
                                    color: "transparent"
                                    Label {
                                        anchors.centerIn: parent
                                        text: "Password"
                                        font.family: "Helvetica"
                                        font.pointSize: 14

                                    }
                                }

                                Rectangle {
                                    anchors.fill: password_input_textinput
                                    color: "#ffffff"
                                }

                                TextInput {
                                    id: password_input_textinput
                                    anchors.verticalCenter: parent.verticalCenter
                                    anchors.left: password_input_label.right
                                    anchors.leftMargin: 10
                                    anchors.right: parent.right
                                    font.family: "Helvetica"
                                    font.pointSize: 13
                                    validator: RegExpValidator { regExp: /[0-9a-zA-Z]{0,22}/ }
                                    echoMode: TextInput.Password
                                }
                            }
                        }

                        Rectangle {
                            id: btns
                            anchors.left: parent.left
                            anchors.right: parent.right
                            anchors.bottom: parent.bottom
                            height: 70
                            color: "transparent"

                            Rectangle {
                                id: log_in_dialog_btn
                                anchors.top: parent.top
                                anchors.left: parent.left
                                anchors.bottom: parent.bottom
                                width: parent.width / 2

                                color: "transparent"
                                Button {
                                    anchors.centerIn: parent
                                    width: parent.width / 4 * 3

                                    text: qsTr("LogIn")

                                    onClicked: {
                                        if (account_input_textinput.length < 6)
                                        {
                                            msg_box.showMsgBox("Error", "The length of account ID may not less than 6");

                                        }
                                        else if (password_input_textinput.length < 6)
                                        {
                                            msg_box.showMsgBox("Error", "The length of password may not less than 6");
                                        }
                                        else
                                        {
                                            if (!socket.is_connected)
                                            {
                                                if (!socket.connect())
                                                {
                                                    msg_box.showMsgBox("Connection Error", "Failed to connect to the server.");
                                                }

                                            }
                                            if (socket.is_connected)
                                            {
                                                var obj = {"EventGroup": "Account","Event": "LogIn","ID": account_input_textinput.text,"Pswd": password_input_textinput.text}
                                                socket.setCallBack(content_item.received);
                                                socket.write(JSON.stringify(obj))
                                            }
                                        }
                                    }
                                }
                            }

                            Rectangle {
                                id: close_dialog_btn
                                anchors.top: parent.top
                                anchors.left: log_in_dialog_btn.right
                                anchors.bottom: parent.bottom
                                anchors.right: parent.right

                                color: "transparent"
                                Button {
                                    anchors.centerIn: parent
                                    width: parent.width / 4 * 3

                                    text: qsTr("Cancel")

                                    onClicked: log_in_dialog.close();
                                }
                            }
                        }
                    }


                }

            }

        }
    }

    Dialog {
        id: msg_box
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: parent

        title: " "
        Label {
            id: msg_info
            font.family: "Helvetica"
        }

        function showMsgBox(title, text)
        {
            msg_box.title = title
            msg_info.text = text
            msg_box.open()
        }
    }

    TcpSocket {
        id: socket
        host: "localhost"
        port: 12345

        property var response: ""
        property var call_back
        property bool is_connected: false

        onConnected: is_connected = true;
        onDisconnected: is_connected = false;

        onRead: {
            console.log(message);
            response = message;
            call_back();
            // content_item.received();
        }

        function setCallBack(call_back_func)
        {
            call_back = call_back_func;
        }
    }

    onClosing: socket.disconnect();
}
