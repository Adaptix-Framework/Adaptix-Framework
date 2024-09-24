#include <UI/Widgets/AdaptixWidget.h>

bool AdaptixWidget::isValidSyncPacket(QJsonObject jsonObj)
{
    if ( !jsonObj.contains("type") || !jsonObj["type"].isDouble() || !jsonObj.contains("subtype") || !jsonObj["subtype"].isDouble() )
        return false;

    int spType    = jsonObj["type"].toDouble();
    int spSubType = jsonObj["subtype"].toDouble();

    if( spType == TYPE_SYNC ) {
        if( spSubType == TYPE_SYNC_START ) {
            if ( !jsonObj.contains("count") || !jsonObj["count"].isDouble() ) {
                return false;
            }
        }
        return true;
    }

    if( spType == TYPE_LISTENER ) {
        if( spSubType == TYPE_LISTENER_NEW ) {
            if ( !jsonObj.contains("fn") || !jsonObj["fn"].isString() ) {
                return false;
            }
            if ( !jsonObj.contains("ui") || !jsonObj["ui"].isString() ) {
                return false;
            }
            return true;
        }

        if( spSubType == TYPE_LISTENER_START ) {
            if ( !jsonObj.contains("l_name") || !jsonObj["l_name"].isString() ) {
                return false;
            }
            if ( !jsonObj.contains("l_type") || !jsonObj["l_type"].isString() ) {
                return false;
            }
            if ( !jsonObj.contains("l_bind_host") || !jsonObj["l_bind_host"].isString() ) {
                return false;
            }
            if ( !jsonObj.contains("l_bind_port") || !jsonObj["l_bind_port"].isString() ) {
                return false;
            }
            if ( !jsonObj.contains("l_agent_host") || !jsonObj["l_agent_host"].isString() ) {
                return false;
            }
            if ( !jsonObj.contains("l_agent_port") || !jsonObj["l_agent_port"].isString() ) {
                return false;
            }
            if ( !jsonObj.contains("l_status") || !jsonObj["l_status"].isString() ) {
                return false;
            }
            return true;
        }

        if( spSubType == TYPE_LISTENER_STOP ) {
            if ( !jsonObj.contains("l_name") || !jsonObj["l_name"].isString() ) {
                return false;
            }
            return true;
        }
    }

    if ( !jsonObj.contains("time") || !jsonObj["time"].isDouble() )
        return false;

    if( spType == TYPE_CLIENT ) {
        if ( !jsonObj.contains("username") || !jsonObj["username"].isString() ) {
            return false;
        }
        return true;
    }

    return false;
}

void AdaptixWidget::processSyncPacket(QJsonObject jsonObj)
{
    int spType    = jsonObj["type"].toDouble();
    int spSubType = jsonObj["subtype"].toDouble();

    if( dialogSyncPacket != nullptr ) {
        dialogSyncPacket->receivedLogs++;
        dialogSyncPacket->upgrade();
    }

    switch (spType) {

        case TYPE_SYNC:

            if( spSubType == TYPE_SYNC_START)
            {
                int count = jsonObj["count"].toDouble();
                dialogSyncPacket = new DialogSyncPacket(count);
            }
            else if (spSubType == TYPE_SYNC_FINISH)
            {
                if (dialogSyncPacket != nullptr ) {
                    dialogSyncPacket->finish();
                    delete dialogSyncPacket;
                    dialogSyncPacket = nullptr;
                }
            }
            break;

        case TYPE_CLIENT:

            if( spSubType == TYPE_CLIENT_CONNECT )
            {
                qint64 time = static_cast<qint64>(jsonObj["time"].toDouble());
                QString username = jsonObj["username"].toString();
                QString message = QString("Client '%1' connected to teamserver").arg(username);

                LogsTab->AddLogs(spSubType, time, message);
            }
            else if ( spSubType == TYPE_CLIENT_DISCONNECT )
            {
                qint64 time = static_cast<qint64>(jsonObj["time"].toDouble());
                QString username = jsonObj["username"].toString();
                QString message = QString("Client '%1' disconnected from teamserver").arg(username);

                LogsTab->AddLogs(spSubType, time, message);
            }
            break;

        case TYPE_LISTENER:
            if( spSubType == TYPE_LISTENER_NEW )
            {
                QString fn = jsonObj["fn"].toString();
                QString ui = jsonObj["ui"].toString();

                RegisterListeners[fn] = new WidgetBuilder(ui.toLocal8Bit() );
            }
            else if ( spSubType == TYPE_LISTENER_START )
            {
                ListenerData newListener = {0};
                newListener.ListenerName = jsonObj["l_name"].toString();
                newListener.ListenerType = jsonObj["l_type"].toString();
                newListener.BindHost     = jsonObj["l_bind_host"].toString();
                newListener.BindPort     = jsonObj["l_bind_port"].toString();
                newListener.AgentPort    = jsonObj["l_agent_port"].toString();
                newListener.AgentHost    = jsonObj["l_agent_host"].toString();
                newListener.Status       = jsonObj["l_status"].toString();

                qint64 time = static_cast<qint64>(jsonObj["time"].toDouble());
                QString message = QString("Listener '%1' (%2) started").arg(newListener.ListenerName).arg(newListener.ListenerType);

                LogsTab->AddLogs(spSubType, time, message);

                ListenersTab->AddListenerItem(newListener);
            }
            else if ( spSubType == TYPE_LISTENER_STOP )
            {
                auto listenerName = jsonObj["l_name"].toString();

                qint64 time = static_cast<qint64>(jsonObj["time"].toDouble());
                QString message = QString("Listener '%1' stopped").arg(listenerName);

                LogsTab->AddLogs(spSubType, time, message);

                ListenersTab->RemoveListenerItem(listenerName);
            }
            break;
    }
}