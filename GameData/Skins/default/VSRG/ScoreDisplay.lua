ScoreDisplay = ScoreDisplay or {}

ScoreDisplay.Digits = {}
ScoreDisplay.DigitWidth = 30
ScoreDisplay.Sheet = "VSRG/combosheet.csv"
ScoreDisplay.DigitHeight = 30
ScoreDisplay.DigitCount = 9
ScoreDisplay.W = ScoreDisplay.DigitWidth * ScoreDisplay.DigitCount
ScoreDisplay.H = ScoreDisplay.DigitHeight
ScoreDisplay.X = ScreenWidth - ScoreDisplay.W -- Topleft
ScoreDisplay.Y = ScreenHeight - ScoreDisplay.H
ScoreDisplay.Targets = {}
ScoreDisplay.Images = {}

function ScoreDisplay.SetName(i)
	return i-1 .. ".png"
end

function ScoreDisplay.Init()

	ScoreDisplay.Score = 0
	ScoreDisplay.DisplayScore = 0

	for i = 1, 10 do -- Digit images
		ScoreDisplay.Images[i] = ScoreDisplay.SetName(i)
	end

	ScoreDisplay.Atlas = TextureAtlas:new(Obj.GetSkinFile(ScoreDisplay.Sheet))

	for i = 1, ScoreDisplay.DigitCount do
		ScoreDisplay.Targets[i] = Obj.CreateTarget()
		Obj.SetTarget(ScoreDisplay.Targets[i])
		Obj.SetPosition(ScoreDisplay.X + ScoreDisplay.W - ScoreDisplay.DigitWidth * i, 
							ScoreDisplay.Y + ScoreDisplay.H - ScoreDisplay.DigitHeight)
		Obj.SetImageSkin("VSRG/"..ScoreDisplay.Atlas.File)
		Obj.SetSize(ScoreDisplay.DigitWidth, ScoreDisplay.DigitHeight)

		local Tab = ScoreDisplay.Atlas.Sprites[ScoreDisplay.Images[1]]

		Obj.CropByPixels(Tab.x, Tab.y, Tab.x+Tab.w, Tab.y+Tab.h)

		Obj.SetAlpha(1)
	end
end

function ScoreDisplay.Run(Delta)
	ScoreDisplay.DisplayScore = math.min((ScoreDisplay.Score - ScoreDisplay.DisplayScore) * Delta * 70 + ScoreDisplay.DisplayScore, ScoreDisplay.Score)
	ScoreDisplay.Digits = {}

	local TCombo = math.ceil(ScoreDisplay.DisplayScore)
	local tdig = 0

	while TCombo >= 1 do
		table.insert(ScoreDisplay.Digits, math.floor(TCombo) % 10)
		TCombo = TCombo / 10
		tdig = tdig + 1
	end

	for i=1, tdig do
		Obj.SetTarget(ScoreDisplay.Targets[i])

		local Tab = ScoreDisplay.Atlas.Sprites[ScoreDisplay.Images[ScoreDisplay.Digits[i]+1]]
		Obj.CropByPixels(Tab.x, Tab.y, Tab.x+Tab.w, Tab.y+Tab.h)
		Obj.SetSize(ScoreDisplay.DigitWidth, ScoreDisplay.DigitHeight)
		Obj.SetAlpha(1)

	end

	for i=tdig+1, ScoreDisplay.DigitCount do

		Obj.SetTarget(ScoreDisplay.Targets[i])

		local Tab = ScoreDisplay.Atlas.Sprites[ScoreDisplay.Images[1]]

		Obj.CropByPixels(Tab.x, Tab.y, Tab.x+Tab.w, Tab.y+Tab.h)
		Obj.SetSize(ScoreDisplay.DigitWidth, ScoreDisplay.DigitHeight)
		Obj.SetAlpha(1)
	end

end

function ScoreDisplay.Update()
	ScoreDisplay.Score = SCScore
end

function ScoreDisplay.Cleanup()

	for i = 1, ScoreDisplay.DigitCount do
		Obj.CleanTarget(ScoreDisplay.Targets[i])
	end

end
