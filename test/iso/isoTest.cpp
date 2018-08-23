#include "isoTest.h"
#include "fixrgraphiso/isomorphismClass.h"
#include "fixrgraphiso/serialization.h"
#include "fixrgraphiso/acdfg.h"

namespace isotest {
  using std::string;
  using std::cout;

  using fixrgraphiso::Acdfg;
  using fixrgraphiso::MethodNode;
  using fixrgraphiso::AcdfgSerializer;
  using fixrgraphiso::IsoSubsumption;

  namespace iso_protobuf = edu::colorado::plv::fixr::protobuf;

  IsoTest::IsoTest()
  {
  }

  TEST_P(IsoTest, ByDefaultIsoIsTrue) {
    string const& fileName = GetParam();
    AcdfgSerializer s;
    vector<MethodNode*> targets;

    iso_protobuf::Acdfg * proto_acdfg = s.read_protobuf_acdfg(fileName.c_str());
    Acdfg * acdfg = s.create_acdfg((const iso_protobuf::Acdfg&) *proto_acdfg);
    delete(proto_acdfg);

    acdfg->getMethodNodes(targets);
    Acdfg * slicedAcdfg = acdfg->sliceACDFG(targets);
    delete(acdfg);

    IsoSubsumption d(slicedAcdfg, slicedAcdfg);
    bool result = d.check();

    EXPECT_EQ(result, true);

    delete(slicedAcdfg);
  }

  TEST_P(IsoTest, SerializeAcdfg) {
    string const& inFile = GetParam();
    string const& outFile = "./out.acdfg.bin";

    AcdfgSerializer s;

    Acdfg * orig_acdfg;
    Acdfg * read_acdfg;

    {
      iso_protobuf::Acdfg * proto_acdfg = s.read_protobuf_acdfg(inFile.c_str());
      orig_acdfg = s.create_acdfg((const iso_protobuf::Acdfg&) *proto_acdfg);
      delete(proto_acdfg);
    }

    {
      // serialize to proto
      iso_protobuf::Acdfg * proto_acdfg = new iso_protobuf::Acdfg();
      s.fill_proto_from_acdfg((const Acdfg&) *orig_acdfg, proto_acdfg);

      // read back the acdfg
      read_acdfg = s.create_acdfg((const iso_protobuf::Acdfg&) *proto_acdfg);

      delete(proto_acdfg);
    }

    bool isEqual = (*orig_acdfg) == (*read_acdfg);

    EXPECT_EQ(isEqual, true);

    delete(orig_acdfg);
    delete(read_acdfg);
  }

  INSTANTIATE_TEST_CASE_P(InstantiationName, IsoTest, ::testing::Values(
    "../test_data/com.dagwaging.rosewidgets.netreg.widget.UpdateService_update.acdfg.bin",
    "../test_data/com.swijaya.android.rotatecontrol.RotateControlWidgetProvider_createWidgetWithState.acdfg.bin",
    "../test_data/com.wzystal.dynamicloader.PluginsWidgetService$GridRemoteViewsFactory_getViewAt.acdfg.bin",
    "../test_data/com.commonsware.android.appwidget.lorem.LoremViewsFactory_getViewAt.acdfg.bin",
    "../test_data/com.rocketmaso.maltine.PlayerService_createAppWidget.acdfg.bin",
    "../test_data/com.matoski.adbm.service.ManagerService_updateWidgets.acdfg.bin",
    "../test_data/org.xbmc.android.remote.presentation.controller.AppWidgetRemoteController_setupWidgetButton.acdfg.bin",
    "../test_data/com.nolanlawson.apptracker.WidgetUpdater_buildUpdate.acdfg.bin",
    "../test_data/com.codeskraps.lolo.home.LoloProvider_updateWidget.acdfg.bin",
    "../test_data/mobiric.demo.wifiwidget.WifiWidgetService$WidgetUpdateThread_run.acdfg.bin",
    "../test_data/me.openphoto.android.app.service.UploaderService_showUploadNotification.acdfg.bin",
    "../test_data/org.metawatch.manager.MetaWatchService_createNotification.acdfg.bin",
    "../test_data/ada.bilkent.transport.BilkentTransportWidgetConfigure$2_onClick.acdfg.bin",
    "../test_data/com.chinaece.gaia.service.UpdateService_UpdateWidget.acdfg.bin",
    "../test_data/org.kreed.vanilla.OneCellWidget_updateWidget.acdfg.bin",
    "../test_data/edu.feri.jager.soslokator.widget.MyWidgetProvider_updateView.acdfg.bin",
    "../test_data/x.x.x.AnalogClockWidget_onReceive.acdfg.bin",
    "../test_data/gingermodupdaterapp.service.DownloadService_onProgressUpdate.acdfg.bin",
    "../test_data/b.r.b.HomeScreenActivity_disableMessage.acdfg.bin",
    "../test_data/com.pongal.aathichudi.AathichudiAppWidgetProvider_onUpdate.acdfg.bin",
    "../test_data/com.mirrorlabs.widgets.UpdateWidgetService_onStart.acdfg.bin",
    "../test_data/com.ccf.feige.activity.MyFeiGeBaseActivity_onCreate.acdfg.bin",
    "../test_data/org.uguess.android.sysinfo.WidgetProvider_update.acdfg.bin",
    "../test_data/com.ldw.music.service.MediaService_updateNotification.acdfg.bin",
    "../test_data/org.kreed.vanilla.FourLongWidget_updateWidget.acdfg.bin",
    "../test_data/com.racoon.ampdroid.Mp3PlayerService_setNotifiction.acdfg.bin",
    "../test_data/org.fairphone.peaceofmind.widget.WidgetProvider_updateUI.acdfg.bin",
    "../test_data/b.r.b.Widget_onUpdate.acdfg.bin",
    "../test_data/com.dagwaging.rosewidgets.db.widget.UpdateService_update.acdfg.bin",
    "../test_data/com.zhaoshouren.android.apps.clock.service.DigitalClockService_onStartCommand.acdfg.bin",
    "../test_data/com.swijaya.galaxytorch.GalaxyTorchWidgetProvider_onUpdate.acdfg.bin"
    )
  );
}
