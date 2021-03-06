HitLightning = {}

HitLightning.OffTime = {}

HitLightning.Times = {}

HitLightning.Height = 250

function LightFunction () if GetConfigF("DisableHitlightningAnimation", "") == 1 then return 0 else return 1 end end

HitLightning.Enabled = GetConfigF("DisableHitlightning", "") ~= 0
HitLightning.Animate = LightFunction()

HitLightning.Pressed = {}
HitLightning.Position = {}

HitLightning.Image = "VSRG/hitlightning.png"

function HitLightning.Init()

	if HitLightning.Enabled == 0 then
		return
	end

	for i = 1, Channels do
		HitLightning.Times[i] = 1
		HitLightning.Pressed[i] = 0
	end

	for i=1, Channels do
		HitLightning[i] = Obj.CreateTarget()

		HitLightning.OffTime[i] = 1

		Obj.SetTarget(HitLightning[i])
		Obj.SetImageSkin(HitLightning.Image)
		Obj.SetCentered(1)
		Obj.SetBlendMode(BlendAdd)

		Obj.SetSize(GetConfigF("Key" .. i .. "Width", ChannelSpace), HitLightning.Height)

		local scrollY = 0
		local scrollX = 0
		if Upscroll ~= 0 then
			scrollX = GetConfigF("Key" .. i .. "X", ChannelSpace);
			scrollY = GearHeight +  HitLightning.Height / 2
			Obj.SetRotation(180)
		else
			scrollX = GetConfigF("Key" .. i .. "X", ChannelSpace);
			scrollY = ScreenHeight - GearHeight - HitLightning.Height / 2
		end

		HitLightning.Position[i] = { x = scrollX, y = scrollY }
		Obj.SetPosition(scrollX, scrollY)
		Obj.SetZ(15)
		Obj.SetAlpha(0)
	end
end

function HitLightning.LanePress(Lane, IsKeyDown, SetRed)

	if HitLightning.Enabled == 0 then
		return
	end

	if CurrentSPB ~= math.huge then
		HitLightning.OffTime[Lane+1] = CurrentSPB / 1.5
	end

	if HitLightning.OffTime[Lane+1] > 3 then
		HitLightning.OffTime[Lane+1] = 3
	end	

	if IsKeyDown == 0 then
		HitLightning.Times[Lane+1] = 0
		HitLightning.Pressed[Lane+1] = 0
	else
		HitLightning.Pressed[Lane+1] = 1
	end
end

function HitLightning.Run(Delta)

	if HitLightning.Enabled == 0 then
		return
	end

	for i=1, Channels do

		HitLightning.Times[i] = HitLightning.Times[i] + Delta

		Obj.SetTarget(HitLightning[i])

		if HitLightning.Pressed[i] == 0 then
			if HitLightning.Times[i] <= HitLightning.OffTime[i] then
				if HitLightning.Animate == 1 then
					local Lerping = math.pow(HitLightning.Times[i] / HitLightning.OffTime[i], 2)
					local Additive
					Obj.SetScale(1 - Lerping, 1 + 1.5 * Lerping)

					Additive = HitLightning.Height / 2 * 1.5 * Lerping

					if Upscroll ~= 0 then
						Additive = Additive * -1
					end

					Obj.SetPosition( HitLightning.Position[i].x, HitLightning.Position[i].y - Additive)
					Obj.SetAlpha( 1 - Lerping )
				elseif HitLightning.Animate == 2 then
					local Lerping = math.pow(HitLightning.Times[i] / HitLightning.OffTime[i], 2)
					local Additive = 0

					Additive = HitLightning.Height / 2 * Lerping
					Obj.SetScale(1, 1 - Lerping)
					Obj.SetPosition( HitLightning.Position[i].x, HitLightning.Position[i].y + Additive)
					Obj.SetAlpha(1)
				end
			else
				Obj.SetAlpha( 0 )
			end

		else
			Obj.SetTarget(HitLightning[i])
			Obj.SetScale( 1, 1 )
			Obj.SetPosition( HitLightning.Position[i].x, HitLightning.Position[i].y)
			Obj.SetAlpha ( 1 )
		end

	end
end

function HitLightning.Cleanup()

	if HitLightningEnabled == 0 then
		return
	end

	for i=1, Channels do
		Obj.CleanTarget(HitLightning[i])
	end

end
