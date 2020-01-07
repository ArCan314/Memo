import QtQuick 2.12
import QtQuick.Controls 2.12

import com.ac.socket 1.0

Component
{
    ItemDelegate
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
                if (checked == true)
                {
                    edit.readOnly = true;
                    edit.font.strikeout = true;
                }
                else
                {
                    edit.readOnly = false;
                    edit.font.strikeout = false;
                    if (downsize.checked == true)
                    {
                        edit.focus = true;
                    }
                }
            }
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

        RoundButton {
            id: downsize
            property bool now_state: false
            antialiasing: true
            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.top: parent.top
            anchors.topMargin: 10

            Image {
                anchors.centerIn: parent
                anchors.topMargin: 8
                antialiasing: true
                width: parent.width / 16 * 9
                height: parent.height / 16 * 9
                source: "qrc:/down_arrow.svg"
                rotation : parent.now_state ? -90 : 0
            }

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
                visible: false;
                width: flick.width
                height: flick.height
                selectByMouse: true
                activeFocusOnPress: false
                cursorVisible: false
                textFormat: TextEdit.PlainText
                focus: false
                text: "test"
                font.family: "Microsoft YaHei"
                font.pointSize: 20
                wrapMode: TextEdit.Wrap

                onCursorRectangleChanged: flick.ensureVisible(cursorRectangle)
                onLengthChanged: {
                    var before_position = cursorPosition > 120 ? 120 : cursorPosition;
                    if (length > 120)
                    {
                        text = text.substring(0, 120);
                        cursorPosition = before_position;
                    }
                }
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
            text: String(120 - edit.length)
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

