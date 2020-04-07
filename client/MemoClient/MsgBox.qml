import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {
    id: msg_box
    modal: true
    standardButtons: Dialog.Ok
    parent: Overlay.overlay
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
