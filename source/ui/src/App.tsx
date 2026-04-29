import { OscSection } from './components/OscSection.js'
import { GlideSection } from './components/GlideSection.js'
import { FilterSection } from './components/FilterSection.js'
import { EnvSection } from './components/EnvSection.js'
import { LfoSection } from './components/LfoSection.js'
import { GlobalSection } from './components/GlobalSection.js'

export default function App() {
  return (
    <div className="w-full h-screen bg-zinc-900 text-zinc-200 flex flex-col overflow-hidden select-none">
      <div className="flex items-center px-3 py-1.5 border-b border-zinc-700/60">
        <span className="text-sm font-mono font-bold tracking-[0.2em] text-zinc-300">Synple</span>
      </div>

      <div className="flex-1 flex flex-col gap-2 p-2 overflow-auto">
        {/* Top row */}
        <div className="flex gap-2 flex-wrap">
          <OscSection />
          <GlideSection />
          <FilterSection />
        </div>

        {/* Middle row */}
        <div className="flex gap-2 flex-wrap">
          <EnvSection prefix="env" label="Amp Env" />
          <EnvSection prefix="filter" label="Filter Env" />
        </div>

        {/* Bottom row */}
        <div className="flex gap-2 flex-wrap">
          <LfoSection />
          <GlobalSection />
        </div>
      </div>
    </div>
  )
}
