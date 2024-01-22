Scriptname SKPEPE_PushEffect extends ActiveMagicEffect Hidden 
{Knocks down and pushes the target with force equal to magnitude}

float mag

Event OnInit()
	mag = GetMagnitude()
EndEvent

Event OnEffectStart(actor Target, actor Caster)
	Caster.PushActorAway(Target, mag)
EndEvent