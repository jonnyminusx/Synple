import { useRef, useEffect } from 'react'
import { getNativeFunction } from './lib/juce/index.js'
import { MidiLearnProvider } from './context/MidiLearnContext.js'
import { Osc1Section } from './components/OscSection.js'
import { Osc2Section } from './components/Osc2Section.js'
import { GlideSection } from './components/GlideSection.js'
import { FilterSection } from './components/FilterSection.js'
import { EnvSection } from './components/EnvSection.js'
import { LfoSection } from './components/LfoSection.js'
import { GlobalSection } from './components/GlobalSection.js'

const setWindowSize = getNativeFunction('setWindowSize')

export default function App() {
  const contentRef = useRef<HTMLDivElement>(null)

  useEffect(() => {
    requestAnimationFrame(() => {
      if (!contentRef.current) return
      const rect = contentRef.current.getBoundingClientRect()
      // Add the p-2 (8px) padding on each side that wraps the content
      const w = Math.ceil(rect.right) + 8
      const h = Math.ceil(rect.bottom) + 8
      if (w > 0 && h > 0) setWindowSize(w, h)
    })
  }, [])

  return (
    <MidiLearnProvider>
    <div style={{ zoom: 1.2 }} className="w-full h-screen bg-zinc-600 text-zinc-200 flex flex-col overflow-hidden select-none">
      <div className="flex items-center justify-between px-3 py-1.5 border-b border-zinc-500/60">
        <span className="text-xl font-mono font-bold tracking-[0.2em] text-zinc-300 uppercase">Synple</span>
        <span className="text-xs font-mono font-bold tracking-[0.15em] text-zinc-500 uppercase">Ghostwritten DSP</span>
      </div>

      <div className="flex-1 flex flex-col gap-2 p-2 overflow-hidden">
        <div ref={contentRef} className="flex flex-col gap-2 w-fit">
          {/* Top row */}
          <div className="flex gap-2 justify-center">
            <Osc1Section />
            <Osc2Section />
            <GlideSection />
            <LfoSection />
          </div>

          {/* Bottom row */}
          <div className="flex gap-2 justify-center">
            <EnvSection prefix="env" label="Amp Env" borderClass="border-cyan-400" bgClass="bg-cyan-200" />
            <EnvSection prefix="filter" label="Filter Env" borderClass="border-blue-400" bgClass="bg-blue-200" />
            <FilterSection />
            <GlobalSection />
          </div>
        </div>
      </div>
    </div>
    </MidiLearnProvider>
  )
}
