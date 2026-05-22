import { useCallback, useRef, useState } from 'react'
import { useJuceSlider } from '../hooks/useJuceSlider.js'
import { useMidiLearn } from '../hooks/useMidiLearn.js'
import { ContextMenu } from './ContextMenu.js'

interface KnobProps {
  parameterId: string
  label: string
  formatValue?: (v: number) => string
}

const SIZE = 36
const CX = SIZE / 2
const CY = SIZE / 2
const RADIUS = 13
const SWEEP = 270
const START_ANGLE = 135 // degrees from 3 o'clock (CSS/SVG convention: 0° = right)

function polarXY(cx: number, cy: number, r: number, angleDeg: number) {
  const rad = (angleDeg * Math.PI) / 180
  return { x: cx + r * Math.cos(rad), y: cy + r * Math.sin(rad) }
}

function arcPath(cx: number, cy: number, r: number, startDeg: number, endDeg: number) {
  const s = polarXY(cx, cy, r, startDeg)
  const e = polarXY(cx, cy, r, endDeg)
  const large = (endDeg - startDeg + 360) % 360 > 180 ? 1 : 0
  return `M ${s.x} ${s.y} A ${r} ${r} 0 ${large} 1 ${e.x} ${e.y}`
}

export function Knob({ parameterId, label, formatValue }: KnobProps) {
  const { scaledValue, normalisedValue, setNormalisedValue, onDragStart, onDragEnd } =
    useJuceSlider(parameterId)
  const { cc, isLearning, startLearn, cancelLearn, clearLearn } = useMidiLearn(parameterId)

  const dragRef = useRef<{ startY: number; startNorm: number } | null>(null)
  const [menu, setMenu] = useState<{ x: number; y: number } | null>(null)

  const handlePointerDown = useCallback(
    (e: React.PointerEvent<SVGSVGElement>) => {
      ; (e.target as Element).setPointerCapture(e.pointerId)
      dragRef.current = { startY: e.clientY, startNorm: normalisedValue }
      onDragStart()
    },
    [normalisedValue, onDragStart],
  )

  const handlePointerMove = useCallback(
    (e: React.PointerEvent<SVGSVGElement>) => {
      if (!dragRef.current) return
      const delta = (dragRef.current.startY - e.clientY) / 120
      setNormalisedValue(Math.max(0, Math.min(1, dragRef.current.startNorm + delta)))
    },
    [setNormalisedValue],
  )

  const handlePointerUp = useCallback(
    (_e: React.PointerEvent<SVGSVGElement>) => {
      dragRef.current = null
      onDragEnd()
    },
    [onDragEnd],
  )

  const handleContextMenu = useCallback((e: React.MouseEvent) => {
    e.preventDefault()
    setMenu({ x: e.clientX, y: e.clientY })
  }, [])

  const menuItems = isLearning
    ? [
        { label: 'Cancel Learning', action: cancelLearn, variant: 'danger' as const },
        ...(cc !== null ? [{ label: `Clear CC ${cc}`, action: clearLearn, variant: 'danger' as const }] : []),
      ]
    : [
        { label: 'Learn MIDI CC', action: startLearn },
        ...(cc !== null ? [{ label: `Clear CC ${cc}`, action: clearLearn, variant: 'danger' as const }] : []),
      ]

  const endAngle = START_ANGLE + normalisedValue * SWEEP
  const dot = polarXY(CX, CY, RADIUS, endAngle)
  const trackEnd = START_ANGLE + SWEEP
  const valueText = formatValue ? formatValue(scaledValue) : scaledValue.toFixed(1)

  return (
    <div
      className="flex flex-col items-center gap-0.5 select-none cursor-ns-resize"
      onContextMenu={handleContextMenu}
    >
      <svg
        width={SIZE}
        height={SIZE}
        viewBox={`0 0 ${SIZE} ${SIZE}`}
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerUp}
        style={isLearning ? { outline: '1.5px solid #f59e0b', outlineOffset: '1px', borderRadius: '50%' } : undefined}
      >
        {/* Track */}
        <path
          d={arcPath(CX, CY, RADIUS, START_ANGLE, trackEnd)}
          fill="none"
          stroke="#c7c7c7"
          strokeWidth="3"
          strokeLinecap="round"
        />
        {/* Value arc */}
        {normalisedValue > 0.001 && (
          <path
            d={arcPath(CX, CY, RADIUS, START_ANGLE, endAngle)}
            fill="none"
            stroke="#464646"
            strokeWidth="3"
            strokeLinecap="round"
          />
        )}
        {/* Dot */}
        <circle cx={dot.x} cy={dot.y} r="2.5" fill="#717171" />
      </svg>
      <span className="text-[9px] text-zinc-800 font-mono leading-none">{valueText}</span>
      <span className="text-[9px] text-zinc-800 font-mono uppercase tracking-wide leading-none">
        {label}
      </span>
      {isLearning && (
        <span className="text-[8px] font-mono text-amber-500 leading-none animate-pulse">learn</span>
      )}
      {!isLearning && cc !== null && (
        <span className="text-[8px] font-mono text-amber-600 leading-none">cc:{cc}</span>
      )}
      {menu && (
        <ContextMenu
          x={menu.x}
          y={menu.y}
          items={menuItems}
          onClose={() => setMenu(null)}
        />
      )}
    </div>
  )
}
