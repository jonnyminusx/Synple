import { Section } from './Section.js'
import { Knob } from './Knob.js'

function fmtLfoRate(v: number) {
  const hz = Math.exp(7 * v - 4)
  return hz < 10 ? hz.toFixed(2) + 'Hz' : hz.toFixed(1) + 'Hz'
}

function fmtVibrato(v: number) {
  if (v < 0) return v.toFixed(0) + '%P'
  return (v >= 0 ? '+' : '') + v.toFixed(0) + '%'
}

export function LfoSection() {
  return (
    <Section label="LFO" borderClass="border-cyan-400" bgClass="bg-cyan-200">
      <Knob parameterId="lfoRate" label="Rate" formatValue={fmtLfoRate} />
      <Knob parameterId="vibrato" label="Vib/PWM" formatValue={fmtVibrato} />
    </Section>
  )
}
