import QtQuick 2.12
import QtQuick.Controls 2.12

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




