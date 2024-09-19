class CRF_ManualMarkerClass : GenericEntityClass
{

}

// YOINKED FROM LOBBY, PRAISE SESK

class CRF_ManualMarker : GenericEntity
{
	// Attributes for editing through workbench TODO: proper description
	[Attribute("{D17288006833490F}UI/Textures/Icons/icons_wrapperUI-32.imageset")]
	protected ResourceName m_sImageSet;
	[Attribute("1 1 1 1", UIWidgets.ColorPicker)]
	ref Color m_MarkerColor;
	[Attribute("empty")]
	protected string m_sQuadName;
	[Attribute("5.0")]
	protected float m_fWorldSize;
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBoxMultiline)]
	protected string m_sDescription;
	[Attribute("true")]
	bool m_bUseWorldScale;
	
	[Attribute("0", UIWidgets.ComboBox, "", "", ParamEnumArray.FromEnum(SCR_EGameModeState))]
	ref array<SCR_EGameModeState> m_aHideOnGameModeStates;
	
	[Attribute("")]
	int m_iZOrder;
	
	[Attribute("")]
	bool m_bShowForAnyFaction;
	
	// Internal variables
	Widget m_wRoot;
	SCR_MapEntity m_MapEntity;
	CRF_ManualMarkerComponent m_hManualMarkerComponent;
	protected ResourceName m_sMarkerPrefab = "{52CA8FF5F56C6F31}UI/layouts/Map/ManualMapMarkerBase.layout";
	
	// Get/Set Broadcast
	// Setters must be call from autority (usualy server)
	string GetImageSet()
	{
		return m_sImageSet;
	}
	void SetImageSet(string imageSet)
	{
		RPC_SetImageSet(imageSet);
		Rpc(RPC_SetImageSet, imageSet);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetImageSet(string imageSet)
	{
		m_sImageSet = imageSet;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) m_hManualMarkerComponent.SetImage(m_sImageSet, m_sQuadName);
	}
	string GetQuadName()
	{
		return m_sQuadName;
	}
	void SetQuadName(string quadName)
	{
		RPC_SetQuadName(quadName);
		Rpc(RPC_SetQuadName, quadName);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetQuadName(string quadName)
	{
		m_sQuadName = quadName;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) 
		{
			m_hManualMarkerComponent.SetImage(m_sImageSet, m_sQuadName);
		}
	}
	
	Color GetColor()
	{
		return m_MarkerColor;
	}
	void SetColor(Color color)
	{
		RPC_SetColor_ByInt(color.PackToInt());
		Rpc(RPC_SetColor_ByInt, color.PackToInt());
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetColor_ByInt(int colorI)
	{
		Color color = Color.FromInt(colorI);
		m_MarkerColor = color;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) m_hManualMarkerComponent.SetColor(m_MarkerColor);
	}
	
	float GetSize()
	{
		return m_fWorldSize;
	}
	void SetSize(float worldSize)
	{
		RPC_SetSize(worldSize);
		Rpc(RPC_SetSize, worldSize);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetSize(float worldSize)
	{
		m_fWorldSize = worldSize;
		
		// Updated on next frame
	}
	
	string GetDescription()
	{
		return m_sDescription;
	}
	void SetDescription(string description)
	{
		RPC_SetDescription(description);
		Rpc(RPC_SetDescription, description);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetDescription(string description)
	{
		m_sDescription = description;
		
		// Update "On fly" if map open and marker exist
		if (m_hManualMarkerComponent) m_hManualMarkerComponent.SetDescription(m_sDescription);
	}
	
	bool GetUseWorldScale()
	{
		return m_bUseWorldScale;
	}
	void SetUseWorldScale(bool useWorldScale)
	{
		RPC_SetUseWorldScale(useWorldScale);
		Rpc(RPC_SetUseWorldScale, useWorldScale);
	}
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_SetUseWorldScale(bool useWorldScale)
	{
		m_bUseWorldScale = useWorldScale;
		
		// Updated on next frame
	}
	
	// Self functions
	override protected void EOnPostFrame(IEntity owner, float timeSlice)
	{
		if (!m_wRoot)
			return;
		
		// Get screen position
		float wX, wY, screenX, screenY, screenXEnd, screenYEnd;
		vector worldPosition = GetOrigin();
		wX = worldPosition[0];
		wY = worldPosition[2];
		m_MapEntity.WorldToScreen(wX, wY, screenX, screenY, true);
		m_MapEntity.WorldToScreen(wX + m_fWorldSize, wY + m_fWorldSize, screenXEnd, screenYEnd, true);

		// Scale to workspace
		float screenXD = GetGame().GetWorkspace().DPIUnscale(screenX);
		float screenYD = GetGame().GetWorkspace().DPIUnscale(screenY);
		float sizeXD = m_fWorldSize;
		float sizeYD = m_fWorldSize;
		if (m_bUseWorldScale) // Calculate world size if need
		{
			sizeXD = GetGame().GetWorkspace().DPIUnscale(screenXEnd - screenX);
			sizeYD = GetGame().GetWorkspace().DPIUnscale(screenY - screenYEnd); // Y flip
		}
		
		int x, y;
		//m_wMarkerIcon.GetImageSize(0, x, y);
		if (y == 0) y = 1;
		
		sizeYD *= (float) y / (float) x;
		
		// Update widget
		// Since every default direction marcers turned to right, -90° added to entity rotation
		m_hManualMarkerComponent.SetSlot(screenXD, screenYD, sizeXD, sizeYD, GetYawPitchRoll()[0] - 90);
	}
	
	override protected void EOnInit(IEntity owner)
	{		
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		ScriptInvokerBase<MapConfigurationInvoker> onMapOpen = m_MapEntity.GetOnMapOpen();
		ScriptInvokerBase<MapConfigurationInvoker> onMapClose = m_MapEntity.GetOnMapClose();
		
		// Every time map open/close add/remove widget
		onMapOpen.Insert(CreateMapWidget);
		onMapClose.Insert(DeleteMapWidget);
	}
	
	// Create new widget in map frame widget
	void CreateMapWidget(MapConfiguration mapConfig)
	{
		// If marker already exists ignore
		if (m_wRoot)
			return;
		
		// Get map frame
		Widget mapFrame = m_MapEntity.GetMapMenuRoot().FindAnyWidget(SCR_MapConstants.MAP_FRAME_NAME);
		if (!mapFrame) mapFrame = m_MapEntity.GetMapMenuRoot();
		if (!mapFrame) return; // Somethig gone wrong
		
		// Create and init marker
		m_wRoot = GetGame().GetWorkspace().CreateWidgets(m_sMarkerPrefab, mapFrame);
		m_wRoot.SetZOrder(m_iZOrder);
		m_hManualMarkerComponent = CRF_ManualMarkerComponent.Cast(m_wRoot.FindHandler(CRF_ManualMarkerComponent));
		m_hManualMarkerComponent.SetImage(m_sImageSet, m_sQuadName);
		m_hManualMarkerComponent.SetDescription(m_sDescription);
		m_hManualMarkerComponent.SetColor(m_MarkerColor);
		m_hManualMarkerComponent.OnMouseLeave(null, null, 0, 0);
		
		// Enable every frame updating
		SetEventMask(EntityEvent.POSTFRAME);
	}
	
	// Delete map widget
	void DeleteMapWidget(MapConfiguration mapConfig)
	{
		// Remove widget if exists
		if (m_wRoot)
			m_wRoot.RemoveFromHierarchy();
		
		// Forget
		m_wRoot = null;
		m_hManualMarkerComponent = null;
		
		// Disable every frame updating
		ClearEventMask(EntityEvent.POSTFRAME);
	}
	
	void CRF_ManualMarker(IEntitySource src, IEntity parent)
	{
		// Enable init event
		SetEventMask(EntityEvent.INIT);
	}
	
	// JIP Replication
	override bool RplSave(ScriptBitWriter writer)
	{
		// Pack every changeable variable
		writer.WriteString(m_sImageSet);
		writer.WriteString(m_sQuadName);
		writer.WriteInt(m_MarkerColor.PackToInt());
		writer.WriteFloat(m_fWorldSize);
		writer.WriteString(m_sDescription);
		writer.WriteBool(m_bUseWorldScale);
		
		return true;
	}
	override bool RplLoad(ScriptBitReader reader)
	{
		// Unpack every changeable variable
		reader.ReadString(m_sImageSet);
		reader.ReadString(m_sQuadName);
		int colorI; // This break flow, where is Color serealization? :(
		reader.ReadInt(colorI);
		m_MarkerColor = Color.FromInt(colorI);
		reader.ReadFloat(m_fWorldSize);
		reader.ReadString(m_sDescription);
		reader.ReadBool(m_bUseWorldScale);
		
		return true;
	}
}

// Custom marker widget component
class CRF_ManualMarkerComponent : SCR_ScriptedWidgetComponent
{
	protected SCR_MapEntity m_MapEntity;
	
	// Cache widgets
	protected ImageWidget m_wMarkerIcon;
	protected ImageWidget m_wMarkerIconGlow;
	protected FrameWidget m_wMarkerFrame;
	protected RichTextWidget m_wDescriptionText;
	protected PanelWidget m_wDescriptionPanel;
	protected OverlayWidget m_wMarkerScrollLayout; // ...
	
	// Internal variables
	protected string m_sDescription;
	protected bool m_bHasGlow;
	protected int m_iZOrder;
	
	// Cache every used widget after attaching to widget tree
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_MapEntity = SCR_MapEntity.GetMapInstance();
		m_wMarkerIcon = ImageWidget.Cast(w.FindAnyWidget("MarkerIcon"));
		m_wMarkerIconGlow = ImageWidget.Cast(w.FindAnyWidget("MarkerGlowIcon"));
		m_wMarkerFrame = FrameWidget.Cast(w.FindAnyWidget("MarkerFrame"));
		m_wDescriptionText = RichTextWidget.Cast(w.FindAnyWidget("DescriptionText"));
		m_wMarkerScrollLayout = OverlayWidget.Cast(w.FindAnyWidget("MarkerScrollLayout"));
		m_wDescriptionPanel = PanelWidget.Cast(w.FindAnyWidget("DescriptionPanel"));
	}
	
	float GetYScale()
	{
		int x, y;
		m_wMarkerIcon.GetImageSize(0, x, y);
		if (y == 0) y = 1;
		float scale = (float) y / (float) x;
		return scale;
	}
	
	// Every info contains in PS_ManualMarker, soo ther is onle setters
	void SetImage(ResourceName m_sImageSet, string quadName)
	{
		if (m_sImageSet.EndsWith(".edds"))
			m_wMarkerIcon.LoadImageTexture(0, m_sImageSet);
		else
			m_wMarkerIcon.LoadImageFromSet(0, m_sImageSet, quadName);
	}
	void SetDescription(string description)
	{
		m_sDescription = description;
		
		m_wDescriptionText.SetText(description);
	}
	void SetColor(Color color)
	{
		m_wMarkerIcon.SetColor(color);	
		m_wMarkerIconGlow.SetColor(color);	
	}
	
	void SetOpacity(float opacity)
	{
		m_wRoot.SetOpacity(opacity);
	}
	
	void SetSlotWorld(vector worldPosition, vector rotation, float worldSize, bool useWorldScale, float minSize = 0.0)
	{
		// Get screen position
		float wX, wY, screenX, screenY, screenXEnd, screenYEnd;
		wX = worldPosition[0];
		wY = worldPosition[2];
		m_MapEntity.WorldToScreen(wX, wY, screenX, screenY, true);
		m_MapEntity.WorldToScreen(wX + worldSize, wY + worldSize, screenXEnd, screenYEnd, true);
		
		// Scale to workspace
		float screenXD = GetGame().GetWorkspace().DPIUnscale(screenX);
		float screenYD = GetGame().GetWorkspace().DPIUnscale(screenY);
		float sizeXD = worldSize;
		float sizeYD = worldSize;
		if (useWorldScale) // Calculate world size if need
		{
			sizeXD = GetGame().GetWorkspace().DPIUnscale(screenXEnd - screenX);
			sizeYD = GetGame().GetWorkspace().DPIUnscale(screenY - screenYEnd); // Y flip
		}
		if (minSize > 0)
		{
			if (sizeXD < minSize) sizeXD = minSize;
			if (sizeYD < minSize) sizeYD = minSize;
		}
		sizeYD *= GetYScale();
		
		SetSlot(screenXD, screenYD, sizeXD, sizeYD, rotation[0] - 90);
	}
	
	// Update marker "Transform", called every frame
	void SetSlot(float posX, float posY, float sizeX, float sizeY, float rotation)
	{
		FrameSlot.SetPos(m_wRoot, posX, posY);
		
		FrameSlot.SetPos(m_wMarkerIcon, -sizeX/2, -sizeY/2);
		FrameSlot.SetSize(m_wMarkerIcon, sizeX, sizeY);
		FrameSlot.SetPos(m_wMarkerIconGlow, -sizeX/2, -sizeY/2);
		FrameSlot.SetSize(m_wMarkerIconGlow, sizeX, sizeY);
		FrameSlot.SetPos(m_wMarkerScrollLayout, -sizeX/2, -sizeY/2);
		FrameSlot.SetSize(m_wMarkerScrollLayout, sizeX, sizeY);
		
		float panelX, panelY;
		m_wDescriptionPanel.GetScreenSize(panelX, panelY);
		float panelXD = GetGame().GetWorkspace().DPIUnscale(panelX);
		float panelYD = GetGame().GetWorkspace().DPIUnscale(panelY);
		if (panelX == 0)
			m_wDescriptionPanel.SetOpacity(0);
		else
			m_wDescriptionPanel.SetOpacity(1);
		FrameSlot.SetPos(m_wDescriptionPanel, -panelXD/2, -panelYD/2);
		m_wMarkerIcon.SetRotation(rotation);
		m_wMarkerIconGlow.SetRotation(rotation);
	}
	*/
};