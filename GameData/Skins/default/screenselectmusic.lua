skin_require("Global/Background.lua")
skin_require("Global/FadeInScreen.lua")
game_require("box.lua")

floor = math.floor

-- Wheel transformation
TransformX =  ScreenWidth * 15/20
CurrentTX = ScreenWidth
WheelSpeed = 12
GDelta = 0

function clamp (v, mn, mx)
    if v < mn then 
	   return mn
	else 
	   if v > mx then  
	    return mx
	   else 
	    return v
	   end
	end
end

-- List transformation
function TransformListHorizontal(Y)
  return ((Y*Y - 768*Y) / (1055)) + CurrentTX
end

function sign(x)
    if x == 0 then return 0 end
    if x > 0 then return 1 else return -1 end
end

Wheel.ListY = 0
Wheel.PendingY = 0
Wheel.ScrollSpeed = 30
LastSign = 0
Time = 0

function TransformPendingVertical(V)
	LastSign = sign(V)
	Time = 0.25
	return V * GDelta * WheelSpeed
end

function TransformListVertical(LY)  

	local Middle = math.ceil(LY) + (ScreenHeight / 4 / Wheel:GetItemHeight())
	local D = Middle % Wheel:GetItemHeight()
	
	Middle = Middle - D
	
	-- Okay, so Middle is the place where we want to lock ourselves, but..

  return LY + (LY - Middle) * clamp(1 - Time / 0.25, 0, 1)
end

function OnItemHover(Index, Line, Selected)
end

function OnItemHoverLeave(Index, Line, Selected)
	local idx = Wheel:IndexAtPoint(ScreenHeight/2)
	Wheel:SetCursorIndex(idx)
end

function OnItemClick(Index, Line, Selected)
	Wheel:SetSelectedItem(Index)
	Wheel.PendingY = (Wheel:NormalizedIndexAtPoint(ScreenHeight/2) - Index) * Wheel:GetItemHeight()
end

function TransformItem(Item, Song, IsSelected)
	if IsSelected == true then
		Item.Image = "itemS.png"
	else
	    local Idx = Wheel:NormalizedIndexAtPoint(Item.Y)
		if Idx == Wheel:GetCursorIndex() then
			Item.Image = "itemH.png"
		else
			Item.Image = "item.png"
		end
	end
end

function OnSongChange(Song, Diff)
	print "Song changed"
	updText(1)
end

function InBanner(frac)
	Obj.SetPosition( 0, -192 * (1-frac) )
	return 1
end

function InUpBtn(frac)
	Obj.SetPosition( ScreenWidth / 2, ScreenHeight + 60 - 81.5 * frac )
	return 1
end

function InBackground(frac)
	Obj.SetAlpha(frac)
	return 1
end

function OutBanner(frac)
	return InBanner(1-frac)
end

function OutUpBtn(frac)
	return InUpBtn(1-frac)
end 

function OutBackground(frac)
	return InBackground(frac)
end

-- Screen Events
function OnSelect()
	TransformX = ScreenWidth + 250
	ScreenFade.In()
	
	Obj.SetTarget(DirUpButton)
	Obj.AddAnimation( "OutUpBtn", 0.5, 0, EaseOut )

	Obj.SetTarget(ScreenBackground)
	Obj.AddAnimation( "OutBackground", 0.5, 0, EaseOut )
	
	return 1
end

function OnRestore()
	ScreenFade.Out()

	TransformX =  ScreenWidth * 15/20
	CurrentTX = ScreenWidth	
	BgAlpha = 0
	Obj.SetTarget(DirUpButton)
	Obj.AddAnimation( "InUpBtn", 0.5, 0, EaseOut )

	Obj.SetTarget(ScreenBackground)
	Obj.AddAnimation( "InBackground", 0.5, 0, EaseOut )
end

function OnDirectoryChange()
	TransformX =  ScreenWidth * 15/20
	CurrentTX = ScreenWidth	+ 250
end

-- ButtonEvents
function DirUpBtnClick()
	CurrentTX = ScreenWidth
end

function KeyEvent(k, c, m)

end

function DirUpBtnHover()
	Obj.SetTarget(DirUpButton)
	Obj.SetImageSkin("SongSelect/up_h.png")
end

function DirUpBtnHoverLeave()
	Obj.SetTarget(DirUpButton)
	Obj.SetImageSkin("SongSelect/up.png")
end

function BackBtnClick()

end

function BackBtnHover()

end

function BackBtnHoverLeave()

end

--[[
function testbox()
	tboxtransform = Transformation()
	tboxtransform.X = 300
	tboxtransform.Y = 200
	testBox = Box:Create({Transform = tboxtransform, Width = 200, Height = 200})
end
]]

function Init()
	BackgroundAnimation:Init()
	ScreenFade.Init()
	
	-- testbox()
	
	Obj.SetTarget(DirUpButton)
	Obj.SetImageSkin("SongSelect/up.png")
	Obj.SetCentered(1)
	Obj.SetZ(18)

	font = Fonts.TruetypeFont(Obj.GetSkinFile("font.ttf"), 24)
		
	dd = StringObject2D()
	dd.Font = font
	dd.Y = 348
	dd.X = 120
	
	Wheel.ScrollSpeed = Wheel:GetItemHeight()
	
	Engine:AddTarget(dd)
	Obj.SetUILayer(30)
end

function updText(t)
	local sng = Global:GetSelectedSong()
	if sng then
		local s7k = toSong7K(sng)
		if s7k then
			local diff = s7k:GetDifficulty(Global.DifficultyIndex)
			if diff then
				local author = diff.Author
				local nps = diff.Objects / diff.Duration
				if string.len(author) > 0 then
					author = " by " .. author
				end

				dd.Text = "Selected " .. diff.Name .. author .. string.format(" (%d of %d)", Global.DifficultyIndex+1, s7k.DifficultyCount) ..
					"\nChannels: " .. diff.Channels .. 
					"\nSong by " .. s7k.Author ..
					"\nLevel " .. diff.Level .. 
					" (" .. string.format("%.02f", nps) .. " nps)"

			end
		else
			dd.Text = "dotcur mode song";
		end
	else
		dd.Text = ""
	end
end

function Cleanup()
end

lastIndex = -1

function Update(Delta)
	GDelta = Delta
	BackgroundAnimation:Update(Delta)
	CurrentTX = CurrentTX + (TransformX - CurrentTX) * Delta * 8

	Time = Time - Delta
	
	local idx = Wheel:NormalizedIndexAtPoint(ScreenHeight/2)
	if lastIndex ~= idx then
	    lastIndex = idx
		if lastIndex ~= Wheel:GetSelectedItem() then
			Wheel:SetCursorIndex(idx)
			Wheel:SetSelectedItem(idx)
		end
	end
	updText()
end
